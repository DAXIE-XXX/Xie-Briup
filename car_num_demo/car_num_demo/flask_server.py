#车牌识别服务器
import os
import json
import base64
from datetime import datetime
import numpy as np
from flask import Flask, request, render_template, jsonify
import cv2
from PIL import Image, ImageDraw, ImageFont
from onnx_project import car_num, Yolo
from models import db, model_info

def draw_chinese_text(img, text, position, font_size=20, color=(0, 255, 0)):
    """在图片上绘制中文文字（解决OpenCV中文乱码问题）"""
    # 将OpenCV图像转换为PIL图像
    img_pil = Image.fromarray(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img_pil)
    
    # 尝试加载中文字体，如果失败则使用默认字体
    font = None
    try:
        # Windows系统字体路径
        if os.name == 'nt':  # Windows系统
            font_paths = [
                'C:/Windows/Fonts/simhei.ttf',  # 黑体
                'C:/Windows/Fonts/simsun.ttc',  # 宋体
                'C:/Windows/Fonts/msyh.ttc',    # 微软雅黑
                'C:/Windows/Fonts/msyhbd.ttc',   # 微软雅黑 Bold
                'C:/Windows/Fonts/simkai.ttf',  # 楷体
            ]
        else:  # Linux/Mac系统
            font_paths = [
                '/usr/share/fonts/truetype/wqy/wqy-microhei.ttc',  # 文泉驿微米黑
                '/usr/share/fonts/truetype/arphic/uming.ttc',       # AR PL UMing
                '/System/Library/Fonts/PingFang.ttc',               # Mac PingFang
            ]
        
        for font_path in font_paths:
            if os.path.exists(font_path):
                try:
                    font = ImageFont.truetype(font_path, font_size)
                    # 成功加载字体，跳出循环
                    break
                except Exception as e:
                    print(f"加载字体 {font_path} 失败: {e}")
                    continue
    except Exception as e:
        print(f"加载字体失败: {e}")
    
    # 如果找不到中文字体，尝试使用PIL的默认字体，但会警告
    if font is None:
        try:
            # 尝试加载一个更通用的字体
            font = ImageFont.load_default()
            print("警告: 使用默认字体，可能无法正确显示中文")
        except:
            font = None
            print("错误: 无法加载任何字体")
    
    # 确保文字是UTF-8编码
    if isinstance(text, bytes):
        text = text.decode('utf-8')
    
    # 绘制文字
    try:
        draw.text(position, text, font=font, fill=color)
    except Exception as e:
        print(f"绘制文字失败: {e}, 文字内容: {text}")
        # 如果绘制失败，尝试使用ASCII编码
        try:
            text_ascii = text.encode('ascii', 'ignore').decode('ascii')
            draw.text(position, text_ascii, font=font, fill=color)
        except:
            pass
    
    # 将PIL图像转换回OpenCV格式
    img_cv = cv2.cvtColor(np.array(img_pil), cv2.COLOR_RGB2BGR)
    return img_cv

app = Flask(__name__)

# SQLite数据库数据初始化
basedir = os.path.abspath(os.path.dirname(__file__))
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///' + os.path.join(basedir, 'models.db')
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False

# 数据库初始化
db.init_app(app)
with app.app_context():
    db.create_all()

# 模型管理器
class ModelManager:
    def __init__(self):
        self.models = {}  # 缓存已加载的模型
    
    def load_model(self, model_dir):
        """加载模型配置和实例"""
        config_path = os.path.join(model_dir, 'config.json')
        if not os.path.exists(config_path):
            return None
        
        with open(config_path, 'r', encoding='utf-8') as f:
            config = json.load(f)
        
        model_type = config.get('model_type')
        model_key = model_dir
        
        if model_key in self.models:
            return self.models[model_key]
        
        if model_type == 'building_yolo':
            # 加载建筑物识别模型
            yolo_filename = config.get('yolo_file')
            if yolo_filename:
                yolo_path = os.path.join(model_dir, yolo_filename)
            else:
                # 兼容旧版本：查找onnx文件
                yolo_files = [f for f in os.listdir(model_dir) if f.endswith('.onnx')]
                if not yolo_files:
                    return None
                yolo_path = os.path.join(model_dir, yolo_files[0])
            
            if not os.path.exists(yolo_path):
                return None
            
            conf = float(config.get('conf', '0.6'))
            iou = float(config.get('iou', '0.5'))
            model = Yolo(yolo_path, confidence_thres=conf, iou_thres=iou)
            self.models[model_key] = {'type': 'building_yolo', 'model': model, 'config': config}
            return self.models[model_key]
        
        elif model_type == 'car_yolo_ocr':
            # 加载车牌识别模型
            yolo_filename = config.get('yolo_file')
            ocr_filename = config.get('ocr_file')
            ocr_dict_filename = config.get('ocr_dict_file')
            
            if yolo_filename and ocr_filename and ocr_dict_filename:
                # 使用配置中的文件名
                yolo_path = os.path.join(model_dir, yolo_filename)
                ocr_path = os.path.join(model_dir, ocr_filename)
                ocr_dict_path = os.path.join(model_dir, ocr_dict_filename)
            else:
                # 兼容旧版本：自动查找文件
                files = os.listdir(model_dir)
                onnx_files = [f for f in files if f.endswith('.onnx')]
                dict_files = [f for f in files if f.endswith('.txt')]
                
                if len(onnx_files) < 2 or not dict_files:
                    return None
                
                yolo_path = None
                ocr_path = None
                for f in onnx_files:
                    if 'yolo' in f.lower():
                        yolo_path = os.path.join(model_dir, f)
                    elif 'ocr' in f.lower():
                        ocr_path = os.path.join(model_dir, f)
                
                if not yolo_path:
                    yolo_path = os.path.join(model_dir, onnx_files[0])
                if not ocr_path and len(onnx_files) > 1:
                    ocr_path = os.path.join(model_dir, onnx_files[1])
                elif not ocr_path:
                    return None
                
                ocr_dict_path = os.path.join(model_dir, dict_files[0])
            
            if not all(os.path.exists(p) for p in [yolo_path, ocr_path, ocr_dict_path]):
                return None
            
            conf = float(config.get('conf', '0.6'))
            iou = float(config.get('iou', '0.5'))
            ocr_shape_str = config.get('ocr_shape', '3,48,320')
            ocr_shape = tuple(int(x) for x in ocr_shape_str.split(','))
            padding = config.get('padding', 'True').lower() == 'true'
            
            model = car_num(
                yolo=yolo_path,
                ocr=ocr_path,
                conf=conf,
                iou=iou,
                ocr_shape=ocr_shape,
                ocr_dict_path=ocr_dict_path,
                padding=padding
            )
            self.models[model_key] = {'type': 'car_yolo_ocr', 'model': model, 'config': config}
            return self.models[model_key]
        
        return None
    
    def list_models(self, model_type=None):
        """列出可用的模型（从文件系统读取，作为数据库的备用方案）"""
        all_models = []
        model_dirs = {
            'building_yolo': 'building_models',
            'car_yolo_ocr': 'car_num_models',
            'handwriting_cnn': 'handwriting_models'
        }
        
        if model_type:
            dirs_to_check = [model_dirs.get(model_type)]
        else:
            dirs_to_check = model_dirs.values()
        
        for base_dir in dirs_to_check:
            if not os.path.exists(base_dir):
                continue
            for folder in os.listdir(base_dir):
                model_dir = os.path.join(base_dir, folder)
                if os.path.isdir(model_dir):
                    config_path = os.path.join(model_dir, 'config.json')
                    if os.path.exists(config_path):
                        with open(config_path, 'r', encoding='utf-8') as f:
                            config = json.load(f)
                        all_models.append({
                            'model_dir': model_dir,
                            'model_name': config.get('model_name', folder),
                            'model_type': config.get('model_type'),
                            'upload_time': config.get('upload_time')
                        })
        
        return all_models

model_manager = ModelManager()


@app.route('/')
def index():
    return render_template('upload_img.html')

@app.route('/building_recognition')
def building_recognition_page():
    """建筑物识别页面"""
    return render_template('building_recognition.html')

@app.route('/car_recognition')
def car_recognition_page():
    """车牌识别页面"""
    return render_template('car_recognition.html')

@app.route('/api/list_models', methods=['GET'])
def list_models_api():
    """获取可用模型列表"""
    model_type = request.args.get('model_type')
    all_models = []
    
    # 优先从数据库读取
    try:
        query = model_info.query
        if model_type:
            query = query.filter_by(model_type=model_type)
        
        db_models = query.order_by(model_info.upload_time.desc()).all()
        
        for db_model in db_models:
            # 验证模型目录是否存在
            if os.path.exists(db_model.model_dir):
                all_models.append({
                    'model_dir': db_model.model_dir,
                    'model_name': db_model.model_name or os.path.basename(db_model.model_dir),
                    'model_type': db_model.model_type,
                    'upload_time': db_model.upload_time.strftime('%Y-%m-%d %H:%M:%S') if db_model.upload_time else None,
                    'model_id': db_model.id
                })
        
        # 如果数据库中有数据，直接返回
        if all_models:
            return jsonify({'status': 'success', 'models': all_models})
    except Exception as e:
        print(f"从数据库读取模型列表失败: {e}，尝试从文件系统读取")
    
    # 如果数据库没有数据或读取失败，从文件系统读取（兼容性）
    models = model_manager.list_models(model_type)
    return jsonify({'status': 'success', 'models': models})

@app.route('/api/building_recognition', methods=['POST'])
def building_recognition():
    """建筑物识别接口"""
    try:
        # 获取模型目录
        model_dir = request.form.get('model_dir')
        if not model_dir:
            return jsonify({'status': 'error', 'message': '请选择模型'}), 400
        
        # 加载模型
        model_data = model_manager.load_model(model_dir)
        if not model_data or model_data['type'] != 'building_yolo':
            return jsonify({'status': 'error', 'message': '模型加载失败或类型不匹配'}), 400
        
        yolo_model = model_data['model']
        
        # 处理图片
        if 'file' not in request.files:
            return jsonify({'status': 'error', 'message': '请上传图片'}), 400
        
        file = request.files['file']
        if file.filename == '':
            return jsonify({'status': 'error', 'message': '文件名为空'}), 400
        
        # 读取图片
        file_bytes = file.read()
        img = cv2.imdecode(np.frombuffer(file_bytes, np.uint8), cv2.IMREAD_COLOR)
        if img is None:
            return jsonify({'status': 'error', 'message': '图片格式错误'}), 400
        
        # 进行检测
        detections = yolo_model.detect_object(img)
        
        # 绘制检测结果
        result_img = img.copy()
        results = []
        for det in detections:
            box = det[0]  # [x, y, w, h]
            score = det[1]
            class_id = det[2]
            
            x1 = int(box[0])
            y1 = int(box[1])
            x2 = int(box[0] + box[2])
            y2 = int(box[1] + box[3])
            
            # 绘制边界框
            cv2.rectangle(result_img, (x1, y1), (x2, y2), (0, 255, 0), 2)
            # 绘制标签（使用中文）
            label = f'建筑物 {class_id}: {score:.2f}'
            result_img = draw_chinese_text(result_img, label, (x1, y1 - 25), font_size=16, color=(0, 255, 0))
            
            results.append({
                'box': [x1, y1, x2, y2],
                'score': float(score),
                'class_id': int(class_id)
            })
        
        # 将结果图片编码为base64
        _, buffer = cv2.imencode('.jpg', result_img)
        img_base64 = base64.b64encode(buffer).decode('utf-8')
        
        return jsonify({
            'status': 'success',
            'detections': results,
            'image': f'data:image/jpeg;base64,{img_base64}',
            'count': len(results)
        })
        
    except Exception as e:
        return jsonify({'status': 'error', 'message': f'识别失败: {str(e)}'}), 500

@app.route('/api/car_recognition', methods=['POST'])
def car_recognition():
    """车牌识别接口"""
    try:
        # 获取模型目录
        model_dir = request.form.get('model_dir')
        if not model_dir:
            return jsonify({'status': 'error', 'message': '请选择模型'}), 400
        
        # 加载模型
        model_data = model_manager.load_model(model_dir)
        if not model_data or model_data['type'] != 'car_yolo_ocr':
            return jsonify({'status': 'error', 'message': '模型加载失败或类型不匹配'}), 400
        
        car_model = model_data['model']
        
        # 处理图片
        if 'file' not in request.files:
            return jsonify({'status': 'error', 'message': '请上传图片'}), 400
        
        file = request.files['file']
        if file.filename == '':
            return jsonify({'status': 'error', 'message': '文件名为空'}), 400
        
        # 读取图片
        file_bytes = file.read()
        img = cv2.imdecode(np.frombuffer(file_bytes, np.uint8), cv2.IMREAD_COLOR)
        if img is None:
            return jsonify({'status': 'error', 'message': '图片格式错误'}), 400
        
        # 进行识别（使用onnx_project.py中的car_num类的get方法）
        # 为了在图片上绘制结果，需要获取检测框，所以直接调用YOLO和OCR
        from onnx_project import Yolo, Ocr
        
        # 获取模型配置
        model_config = model_data['config']
        
        # 从模型目录读取配置文件获取文件路径
        config_path = os.path.join(model_dir, 'config.json')
        if os.path.exists(config_path):
            with open(config_path, 'r', encoding='utf-8') as f:
                config_data = json.load(f)
                yolo_file = config_data.get('yolo_file')
                ocr_file = config_data.get('ocr_file')
                ocr_dict_file = config_data.get('ocr_dict_file')
                
                yolo_path = os.path.join(model_dir, yolo_file) if yolo_file else None
                ocr_path = os.path.join(model_dir, ocr_file) if ocr_file else None
                ocr_dict_path = os.path.join(model_dir, ocr_dict_file) if ocr_dict_file else None
        
        # 如果找不到文件，使用已加载的模型（但无法获取检测框）
        if not yolo_path or not ocr_path or not ocr_dict_path:
            res = car_model.get(img)
            plate_number = ''
            confidence = 0.0
            detected = False
            result_img = img.copy()
            
            if isinstance(res, str):
                plate_number = res
            elif res and len(res) > 0:
                if isinstance(res[0], (list, tuple)) and len(res[0]) >= 2:
                    plate_number = res[0][0]
                    confidence = res[0][1]
                elif isinstance(res[0], (list, tuple)) and len(res[0]) >= 1:
                    plate_number = res[0][0]
                    confidence = 1.0
                else:
                    plate_number = str(res[0])
                    confidence = 1.0
                detected = True
            
            # 将结果图片编码为base64
            _, buffer = cv2.imencode('.jpg', result_img)
            img_base64 = base64.b64encode(buffer).decode('utf-8')
            
            return jsonify({
                'status': 'success',
                'plate_number': plate_number,
                'confidence': float(confidence) if detected else 0.0,
                'detected': detected,
                'image': f'data:image/jpeg;base64,{img_base64}'
            })
        
        # 使用YOLO检测获取检测框
        conf = float(model_config.get('conf', '0.6'))
        iou = float(model_config.get('iou', '0.5'))
        yolo_model = Yolo(yolo_path, confidence_thres=conf, iou_thres=iou)
        yolo_res = yolo_model.detect_object(img)
        
        result_img = img.copy()
        plate_number = ''
        confidence = 0.0
        detected = False
        
        if not yolo_res or len(yolo_res) == 0:
            plate_number = '未检测到车牌'
        else:
            # 获取第一个检测框
            box = yolo_res[0][0]  # [x, y, w, h]
            x1 = int(box[0])
            y1 = int(box[1])
            x2 = int(box[0] + box[2])
            y2 = int(box[1] + box[3])
            
            # 裁剪车牌区域
            car_num_img = img[y1:y2, x1:x2]
            
            # OCR识别
            ocr_shape_str = model_config.get('ocr_shape', '3,48,320')
            ocr_shape = tuple(int(x) for x in ocr_shape_str.split(','))
            padding = model_config.get('padding', 'True').lower() == 'true'
            
            ocr_model = Ocr(
                ocr_path=ocr_path,
                ocr_shape=ocr_shape,
                ocr_dict_path=ocr_dict_path,
                padding=padding
            )
            ocr_res = ocr_model.get_ocr(car_num_img)
            
            # 处理OCR结果
            if isinstance(ocr_res, str):
                plate_number = ocr_res
            elif ocr_res and len(ocr_res) > 0:
                if isinstance(ocr_res[0], (list, tuple)) and len(ocr_res[0]) >= 2:
                    plate_number = ocr_res[0][0]
                    confidence = ocr_res[0][1]
                elif isinstance(ocr_res[0], (list, tuple)) and len(ocr_res[0]) >= 1:
                    plate_number = ocr_res[0][0]
                    confidence = 1.0
                else:
                    plate_number = str(ocr_res[0])
                    confidence = 1.0
                detected = True
                
                # 在图片上绘制检测框和车牌号码
                # 绘制检测框
                cv2.rectangle(result_img, (x1, y1), (x2, y2), (0, 255, 0), 3)
                
                # 绘制车牌号码（使用中文绘制函数，解决乱码问题）
                label = plate_number
                if confidence < 1.0:
                    label += f' ({confidence:.2f})'
                
                # 在图片左上角绘制车牌号码（蓝色）
                result_img = draw_chinese_text(result_img, label, (10, 10), 
                                              font_size=24, color=(0, 0, 255))
        
        # 将结果图片编码为base64
        _, buffer = cv2.imencode('.jpg', result_img)
        img_base64 = base64.b64encode(buffer).decode('utf-8')
        
        return jsonify({
            'status': 'success',
            'plate_number': plate_number,
            'confidence': float(confidence) if detected else 0.0,
            'detected': detected,
            'image': f'data:image/jpeg;base64,{img_base64}'  # 返回带标注的图片
        })
        
    except Exception as e:
        return jsonify({'status': 'error', 'message': f'识别失败: {str(e)}'}), 500

@app.route('/get_model_html')
def get_car_model_html():
    return render_template('upload_model.html')

@app.route('/create_model', methods=["POST"])
def create_model():
    """通用模型上传接口，支持多种模型类型"""
    if request.method == 'POST':
        try:
            data_files = request.files
            data = request.form
            model_type = data.get('model_type')
            model_name = data.get('model_name', '').strip()
            
            if not model_type:
                return jsonify({'status': 'error', 'message': '请选择模型类型'}), 400
            
            # 根据模型类型确定基础目录
            model_type_dirs = {
                'building_yolo': 'building_models',
                'car_yolo_ocr': 'car_num_models',
                'handwriting_cnn': 'handwriting_models'
            }
            
            base_dir = model_type_dirs.get(model_type, 'models')
            os.makedirs(base_dir, exist_ok=True)
            
            # 创建模型文件夹
            timestamp = str(datetime.now()).replace(':', '-').split('.')[0]
            if model_name:
                folder_name = f"{model_name}_{timestamp}"
            else:
                folder_name = f"{model_type}_{timestamp}"
            
            model_dir = os.path.join(base_dir, folder_name)
            os.makedirs(model_dir, exist_ok=True)
            
            config = {
                'model_type': model_type,
                'model_name': model_name if model_name else folder_name,
                'upload_time': timestamp
            }
            
            # 根据模型类型处理不同的文件
            if model_type == 'building_yolo':
                # 建筑物识别：只需要YOLO模型
                # 使用getlist获取文件，因为可能有多个同名字段
                yolo_files = data_files.getlist('yolo_file')
                yolo_file = None
                for f in yolo_files:
                    if f and hasattr(f, 'filename') and f.filename and f.filename.strip():
                        yolo_file = f
                        break
                
                if not yolo_file:
                    return jsonify({'status': 'error', 'message': '请上传YOLO模型文件'}), 400
                
                yolo_path = os.path.join(model_dir, yolo_file.filename)
                yolo_file.save(yolo_path)
                
                conf = data.get('conf', '0.6')
                iou = data.get('iou', '0.5')
                config.update({
                    'yolo_file': yolo_file.filename,
                    'conf': conf,
                    'iou': iou
                })
                
            elif model_type == 'car_yolo_ocr':
                # 车牌识别：需要YOLO、OCR模型和字典文件
                # 使用getlist获取文件，因为可能有多个同名字段
                yolo_files = data_files.getlist('yolo_file')
                ocr_files = data_files.getlist('ocr_file')
                ocr_dict_files = data_files.getlist('ocr_dict_file')
                
                # 从列表中查找有效的文件
                yolo_file = None
                for f in yolo_files:
                    if f and hasattr(f, 'filename') and f.filename and f.filename.strip():
                        yolo_file = f
                        break
                
                ocr_file = None
                for f in ocr_files:
                    if f and hasattr(f, 'filename') and f.filename and f.filename.strip():
                        ocr_file = f
                        break
                
                ocr_dict_file = None
                for f in ocr_dict_files:
                    if f and hasattr(f, 'filename') and f.filename and f.filename.strip():
                        ocr_dict_file = f
                        break
                
                if not yolo_file:
                    return jsonify({'status': 'error', 'message': '请上传YOLO模型文件'}), 400
                if not ocr_file:
                    return jsonify({'status': 'error', 'message': '请上传OCR模型文件'}), 400
                if not ocr_dict_file:
                    return jsonify({'status': 'error', 'message': '请上传OCR字典文件'}), 400
                
                yolo_path = os.path.join(model_dir, yolo_file.filename)
                ocr_path = os.path.join(model_dir, ocr_file.filename)
                ocr_dict_path = os.path.join(model_dir, ocr_dict_file.filename)
                
                yolo_file.save(yolo_path)
                ocr_file.save(ocr_path)
                ocr_dict_file.save(ocr_dict_path)
                
                conf = data.get('conf', '0.6')
                iou = data.get('iou', '0.5')
                ocr_shape = data.get('ocr_shape', '3,48,320')
                padding = data.get('padding', 'True')
                
                config.update({
                    'yolo_file': yolo_file.filename,
                    'ocr_file': ocr_file.filename,
                    'ocr_dict_file': ocr_dict_file.filename,
                    'conf': conf,
                    'iou': iou,
                    'ocr_shape': ocr_shape,
                    'padding': padding
                })
                
            elif model_type == 'handwriting_cnn':
                # 手写识别：只需要CNN模型
                # 使用getlist获取文件，因为可能有多个同名字段
                cnn_files = data_files.getlist('cnn_file')
                cnn_file = None
                for f in cnn_files:
                    if f and hasattr(f, 'filename') and f.filename and f.filename.strip():
                        cnn_file = f
                        break
                
                if not cnn_file:
                    return jsonify({'status': 'error', 'message': '请上传CNN模型文件'}), 400
                
                cnn_path = os.path.join(model_dir, cnn_file.filename)
                cnn_file.save(cnn_path)
                
                input_shape = data.get('input_shape', '28,28')
                num_classes = data.get('num_classes', '10')
                
                config.update({
                    'input_shape': input_shape,
                    'num_classes': num_classes
                })
            else:
                return jsonify({'status': 'error', 'message': '不支持的模型类型'}), 400
            
            # 保存配置文件（JSON格式）
            config_file = os.path.join(model_dir, 'config.json')
            with open(config_file, 'w', encoding='utf-8') as f:
                json.dump(config, f, ensure_ascii=False, indent=2)
            
            # 保存到数据库
            try:
                upload_time = datetime.now()
                new_model = model_info(
                    model_type=model_type,
                    model_name=model_name if model_name else folder_name,
                    model_dir=model_dir,
                    upload_time=upload_time,
                    config_json=json.dumps(config, ensure_ascii=False)
                )
                
                # 根据模型类型保存专用字段
                if model_type == 'building_yolo':
                    new_model.building_yolo_path = yolo_path
                    new_model.building_conf = config.get('conf', '0.6')
                    new_model.building_iou = config.get('iou', '0.5')
                
                elif model_type == 'car_yolo_ocr':
                    new_model.yolo_path = yolo_path
                    new_model.ocr_path = ocr_path
                    new_model.ocr_dict_path = ocr_dict_path
                    new_model.conf = config.get('conf', '0.6')
                    new_model.iou = config.get('iou', '0.5')
                    new_model.ocr_shape = config.get('ocr_shape', '3,48,320')
                    new_model.padding = config.get('padding', 'True')
                
                elif model_type == 'handwriting_cnn':
                    new_model.cnn_path = cnn_path
                    new_model.input_shape = config.get('input_shape', '28,28')
                    new_model.num_classes = config.get('num_classes', '10')
                
                db.session.add(new_model)
                db.session.commit()
                
                return jsonify({
                    'status': 'success',
                    'message': '模型上传成功并已保存到数据库',
                    'model_dir': model_dir,
                    'model_id': new_model.id,
                    'config': config
                }), 200
                
            except Exception as db_error:
                db.session.rollback()
                # 即使数据库保存失败，文件已保存，返回成功但提示数据库错误
                return jsonify({
                    'status': 'warning',
                    'message': f'模型文件保存成功，但数据库保存失败: {str(db_error)}',
                    'model_dir': model_dir,
                    'config': config
                }), 200
            
        except Exception as e:
            return jsonify({'status': 'error', 'message': f'上传失败: {str(e)}'}), 500

if __name__ == '__main__':
    app.run(port=8000)