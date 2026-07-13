"""
车牌识别PyQt应用程序
功能：
1. 调用摄像头或上传图片进行车牌识别
2. 车牌号数据库比对
3. 车牌注册功能
4. 模型选择和管理
"""

import sys
import os
import json
import cv2
import numpy as np
from datetime import datetime
import requests
from PyQt5.QtWidgets import (QApplication, QMainWindow, QWidget, QVBoxLayout, 
                             QHBoxLayout, QPushButton, QLabel, QFileDialog, 
                             QMessageBox, QLineEdit, QTextEdit, QGroupBox,
                             QTableWidget, QTableWidgetItem, QHeaderView,
                             QComboBox, QSplitter, QFrame)
from PyQt5.QtCore import QTimer, Qt, QThread, pyqtSignal
from PyQt5.QtGui import QImage, QPixmap, QFont, QPalette, QColor
from sqlalchemy import create_engine, Column, Integer, String, DateTime
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker
from onnx_project import car_num, Yolo, Ocr

# 数据库模型
Base = declarative_base()

class RegisteredPlate(Base):
    """注册车牌数据库模型"""
    __tablename__ = 'registered_plates'
    
    id = Column(Integer, primary_key=True, autoincrement=True)
    plate_number = Column(String(50), unique=True, nullable=False)  # 车牌号
    owner_name = Column(String(100))  # 车主姓名
    register_time = Column(DateTime, default=datetime.now)  # 注册时间
    notes = Column(String(500))  # 备注

# 数据库初始化
DATABASE_URL = 'sqlite:///registered_plates.db'
engine = create_engine(DATABASE_URL, echo=False)
Base.metadata.create_all(engine)
Session = sessionmaker(bind=engine)


class CameraThread(QThread):
    """摄像头线程"""
    frame_ready = pyqtSignal(np.ndarray)  # 每帧都发送，用于显示
    frame_for_recognition = pyqtSignal(np.ndarray)  # 每10帧发送一次，用于识别
    
    def __init__(self, recognition_interval=10):
        super().__init__()
        self.camera = None
        self.running = False
        self.frame_count = 0
        self.recognition_interval = recognition_interval  # 识别间隔（帧数）
    
    def start_camera(self):
        """启动摄像头"""
        self.camera = cv2.VideoCapture(0)
        if not self.camera.isOpened():
            return False
        self.running = True
        self.frame_count = 0  # 重置帧计数
        self.start()
        return True
    
    def stop_camera(self):
        """停止摄像头"""
        self.running = False
        if self.camera:
            self.camera.release()
    
    def run(self):
        """线程运行"""
        while self.running:
            ret, frame = self.camera.read()
            if ret:
                # 每帧都发送用于显示
                self.frame_ready.emit(frame)
                
                # 每N帧发送一次用于识别
                self.frame_count += 1
                if self.frame_count >= self.recognition_interval:
                    self.frame_for_recognition.emit(frame)
                    self.frame_count = 0  # 重置计数
                    
            self.msleep(30)  # 约30fps


class CarPlateRecognitionApp(QMainWindow):
    """车牌识别主窗口"""
    
    def __init__(self):
        super().__init__()
        self.camera_thread = None
        self.current_image = None
        self.recognition_model = None
        self.db_session = Session()
        self.current_model_dir = None
        self.server_url = "http://localhost:8000"  # 模型服务器地址
        self.auto_recognize_enabled = True  # 自动识别开关
        self.recognition_interval = 10  # 识别间隔（帧数）
        
        # 初始化UI
        self.init_ui()
        
        # 加载模型列表
        self.load_model_list()
        
        # 加载已注册车牌列表
        self.load_registered_plates()
    
    def load_model_list(self):
        """加载模型列表（从本地或服务器）"""
        models = []
        
        # 从本地加载模型
        if os.path.exists('car_num_models'):
            for folder in os.listdir('car_num_models'):
                model_dir = os.path.join('car_num_models', folder)
                config_path = os.path.join(model_dir, 'config.json')
                if os.path.exists(config_path):
                    try:
                        with open(config_path, 'r', encoding='utf-8') as f:
                            config = json.load(f)
                        if config.get('model_type') == 'car_yolo_ocr':
                            models.append({
                                'model_dir': model_dir,
                                'model_name': config.get('model_name', folder),
                                'source': '本地'
                            })
                    except:
                        continue
        
        # 从服务器加载模型
        try:
            response = requests.get(f'{self.server_url}/api/list_models?model_type=car_yolo_ocr', timeout=3)
            if response.status_code == 200:
                data = response.json()
                if data.get('status') == 'success':
                    for model in data.get('models', []):
                        if model.get('model_dir') not in [m['model_dir'] for m in models]:
                            models.append({
                                'model_dir': model.get('model_dir'),
                                'model_name': model.get('model_name', '未知模型'),
                                'source': '服务器'
                            })
        except:
            pass  # 服务器不可用，只使用本地模型
        
        # 更新下拉框
        self.model_combo.clear()
        self.model_combo.addItem('请选择模型', None)
        for model in models:
            display_name = f"{model['model_name']} ({model['source']})"
            self.model_combo.addItem(display_name, model['model_dir'])
        
        # 如果有模型，选择第一个
        if self.model_combo.count() > 1:
            self.model_combo.setCurrentIndex(1)
            self.on_model_changed()
    
    def load_model(self, model_dir=None):
        """加载车牌识别模型"""
        if model_dir is None:
            model_dir = self.current_model_dir
        
        if model_dir is None:
            # 尝试使用默认路径
            if os.path.exists('./yolo_best.onnx') and os.path.exists('./ocr_best.onnx'):
                model_dir = './'
            else:
                return False
        
        try:
            if model_dir == './':
                # 使用默认路径
                yolo_path = './yolo_best.onnx'
                ocr_path = './ocr_best.onnx'
                ocr_dict_path = './ppocr_keys_v1.txt'
                conf = 0.6
                iou = 0.5
                ocr_shape = (3, 48, 320)
                padding = True
            else:
                # 从配置文件加载
                config_path = os.path.join(model_dir, 'config.json')
                if not os.path.exists(config_path):
                    return False
                
                with open(config_path, 'r', encoding='utf-8') as f:
                    config = json.load(f)
                
                yolo_path = os.path.join(model_dir, config.get('yolo_file', 'yolo_best.onnx'))
                ocr_path = os.path.join(model_dir, config.get('ocr_file', 'ocr_best.onnx'))
                ocr_dict_path = os.path.join(model_dir, config.get('ocr_dict_file', 'ppocr_keys_v1.txt'))
                conf = float(config.get('conf', '0.6'))
                iou = float(config.get('iou', '0.5'))
                ocr_shape_str = config.get('ocr_shape', '3,48,320')
                ocr_shape = tuple(int(x) for x in ocr_shape_str.split(','))
                padding = config.get('padding', 'True').lower() == 'true'
            
            # 检查文件是否存在
            missing_files = [p for p in [yolo_path, ocr_path, ocr_dict_path] if not os.path.exists(p)]
            if missing_files:
                QMessageBox.warning(self, '警告', f'模型文件不存在:\n{chr(10).join(missing_files)}')
                return False
            
            # 创建识别模型
            self.recognition_model = car_num(
                yolo=yolo_path,
                ocr=ocr_path,
                conf=conf,
                iou=iou,
                ocr_shape=ocr_shape,
                ocr_dict_path=ocr_dict_path,
                padding=padding
            )
            
            self.current_model_dir = model_dir
            self.model_status_label.setText(f'当前模型: {os.path.basename(model_dir) if model_dir != "./" else "默认模型"}')
            print(f"模型加载成功: {model_dir}")
            return True
            
        except Exception as e:
            QMessageBox.critical(self, '错误', f'加载模型失败: {str(e)}')
            print(f"模型加载失败: {e}")
            return False
    
    def on_model_changed(self):
        """模型选择改变时的回调"""
        model_dir = self.model_combo.currentData()
        if model_dir:
            self.load_model(model_dir)
        else:
            self.recognition_model = None
            self.model_status_label.setText('当前模型: 未选择')
    
    def init_ui(self):
        """初始化用户界面"""
        self.setWindowTitle('车牌识别系统 v2.0')
        self.setGeometry(100, 100, 1400, 900)
        
        # 设置样式
        self.setStyleSheet("""
            QMainWindow {
                background-color: #f5f5f5;
            }
            QGroupBox {
                font-weight: bold;
                border: 2px solid #cccccc;
                border-radius: 5px;
                margin-top: 10px;
                padding-top: 10px;
                background-color: white;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px;
            }
            QPushButton {
                background-color: #4CAF50;
                color: white;
                border: none;
                padding: 8px 16px;
                border-radius: 4px;
                font-size: 14px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #45a049;
            }
            QPushButton:pressed {
                background-color: #3d8b40;
            }
            QPushButton:disabled {
                background-color: #cccccc;
                color: #666666;
            }
            QLineEdit {
                padding: 5px;
                border: 1px solid #cccccc;
                border-radius: 3px;
                font-size: 13px;
            }
            QLineEdit:focus {
                border: 2px solid #4CAF50;
            }
            QComboBox {
                padding: 5px;
                border: 1px solid #cccccc;
                border-radius: 3px;
                font-size: 13px;
            }
            QComboBox:focus {
                border: 2px solid #4CAF50;
            }
            QTableWidget {
                border: 1px solid #cccccc;
                border-radius: 3px;
                background-color: white;
                gridline-color: #e0e0e0;
            }
            QTableWidget::item {
                padding: 5px;
            }
            QTableWidget::item:selected {
                background-color: #4CAF50;
                color: white;
            }
        """)
        
        # 主窗口部件
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QVBoxLayout(central_widget)
        main_layout.setSpacing(10)
        main_layout.setContentsMargins(10, 10, 10, 10)
        
        # 顶部：模型选择区域
        model_group = QGroupBox('模型管理')
        model_layout = QHBoxLayout()
        
        model_layout.addWidget(QLabel('选择模型:'))
        self.model_combo = QComboBox()
        self.model_combo.setMinimumWidth(300)
        self.model_combo.currentIndexChanged.connect(self.on_model_changed)
        model_layout.addWidget(self.model_combo)
        
        refresh_btn = QPushButton('刷新列表')
        refresh_btn.clicked.connect(self.load_model_list)
        model_layout.addWidget(refresh_btn)
        
        model_layout.addStretch()
        
        self.model_status_label = QLabel('当前模型: 未选择')
        self.model_status_label.setStyleSheet("font-size: 12px; color: #666666; padding: 5px;")
        model_layout.addWidget(self.model_status_label)
        
        model_group.setLayout(model_layout)
        main_layout.addWidget(model_group)
        
        # 主内容区域（使用水平分割）
        splitter = QSplitter(Qt.Horizontal)
        
        # 左侧：图像显示和操作区域
        left_widget = QWidget()
        left_layout = QVBoxLayout(left_widget)
        left_layout.setSpacing(10)
        left_layout.setContentsMargins(5, 5, 5, 5)
        
        # 图像显示区域
        image_group = QGroupBox('图像显示')
        image_layout = QVBoxLayout()
        image_layout.setContentsMargins(5, 5, 5, 5)
        self.image_label = QLabel()
        self.image_label.setMinimumSize(640, 480)
        self.image_label.setMaximumSize(1920, 1080)  # 设置最大尺寸
        self.image_label.setScaledContents(False)  # 不自动缩放内容
        self.image_label.setAlignment(Qt.AlignCenter)
        self.image_label.setStyleSheet("""
            QLabel {
                border: 2px solid #4CAF50;
                border-radius: 5px;
                background-color: #f9f9f9;
                color: #666666;
            }
        """)
        self.image_label.setText('请选择图片或启动摄像头')
        image_layout.addWidget(self.image_label)
        image_group.setLayout(image_layout)
        left_layout.addWidget(image_group, 1)  # 设置拉伸因子
        
        # 操作按钮区域
        button_group = QGroupBox('操作')
        button_layout = QHBoxLayout()
        button_layout.setSpacing(10)
        
        self.camera_btn = QPushButton('📷 启动摄像头')
        self.camera_btn.clicked.connect(self.toggle_camera)
        button_layout.addWidget(self.camera_btn)
        
        self.upload_btn = QPushButton('📁 上传图片')
        self.upload_btn.clicked.connect(self.upload_image)
        button_layout.addWidget(self.upload_btn)
        
        self.recognize_btn = QPushButton('🔍 识别车牌')
        self.recognize_btn.clicked.connect(self.recognize_plate)
        self.recognize_btn.setEnabled(False)
        self.recognize_btn.setStyleSheet("""
            QPushButton {
                background-color: #2196F3;
            }
            QPushButton:hover {
                background-color: #0b7dda;
            }
        """)
        button_layout.addWidget(self.recognize_btn)
        
        button_group.setLayout(button_layout)
        left_layout.addWidget(button_group)
        
        # 识别结果区域
        result_group = QGroupBox('识别结果')
        result_layout = QVBoxLayout()
        result_layout.setSpacing(5)
        
        self.result_label = QLabel('识别结果: 未识别')
        self.result_label.setStyleSheet("""
            font-size: 20px;
            font-weight: bold;
            padding: 15px;
            background-color: #e8f5e9;
            border-radius: 5px;
            color: #2e7d32;
        """)
        result_layout.addWidget(self.result_label)
        
        self.status_label = QLabel('状态: 等待操作')
        self.status_label.setStyleSheet("""
            font-size: 14px;
            padding: 10px;
            background-color: #fff3e0;
            border-radius: 5px;
            color: #e65100;
        """)
        result_layout.addWidget(self.status_label)
        
        result_group.setLayout(result_layout)
        left_layout.addWidget(result_group)
        
        left_widget.setLayout(left_layout)
        splitter.addWidget(left_widget)
        
        # 右侧：车牌管理区域
        right_widget = QWidget()
        right_layout = QVBoxLayout(right_widget)
        right_layout.setSpacing(10)
        right_layout.setContentsMargins(5, 5, 5, 5)
        
        # 车牌注册区域
        register_group = QGroupBox('车牌注册')
        register_layout = QVBoxLayout()
        register_layout.setSpacing(8)
        
        plate_input_layout = QHBoxLayout()
        plate_input_layout.addWidget(QLabel('车牌号:'))
        self.plate_input = QLineEdit()
        self.plate_input.setPlaceholderText('请输入车牌号')
        plate_input_layout.addWidget(self.plate_input)
        register_layout.addLayout(plate_input_layout)
        
        owner_input_layout = QHBoxLayout()
        owner_input_layout.addWidget(QLabel('车主姓名:'))
        self.owner_input = QLineEdit()
        self.owner_input.setPlaceholderText('请输入车主姓名（可选）')
        owner_input_layout.addWidget(self.owner_input)
        register_layout.addLayout(owner_input_layout)
        
        notes_input_layout = QHBoxLayout()
        notes_input_layout.addWidget(QLabel('备注:'))
        self.notes_input = QLineEdit()
        self.notes_input.setPlaceholderText('备注信息（可选）')
        notes_input_layout.addWidget(self.notes_input)
        register_layout.addLayout(notes_input_layout)
        
        register_btn_layout = QHBoxLayout()
        register_btn_layout.setSpacing(10)
        self.register_btn = QPushButton('✅ 注册车牌')
        self.register_btn.clicked.connect(self.register_plate)
        register_btn_layout.addWidget(self.register_btn)
        
        self.delete_btn = QPushButton('❌ 删除选中')
        self.delete_btn.setStyleSheet("""
            QPushButton {
                background-color: #f44336;
            }
            QPushButton:hover {
                background-color: #da190b;
            }
        """)
        self.delete_btn.clicked.connect(self.delete_plate)
        register_btn_layout.addWidget(self.delete_btn)
        register_layout.addLayout(register_btn_layout)
        
        register_group.setLayout(register_layout)
        right_layout.addWidget(register_group)
        
        # 已注册车牌列表
        list_group = QGroupBox('已注册车牌列表')
        list_layout = QVBoxLayout()
        
        self.plate_table = QTableWidget()
        self.plate_table.setColumnCount(4)
        self.plate_table.setHorizontalHeaderLabels(['ID', '车牌号', '车主姓名', '注册时间'])
        self.plate_table.horizontalHeader().setStretchLastSection(True)
        self.plate_table.setSelectionBehavior(QTableWidget.SelectRows)
        self.plate_table.setEditTriggers(QTableWidget.NoEditTriggers)
        self.plate_table.setAlternatingRowColors(True)
        list_layout.addWidget(self.plate_table)
        
        list_group.setLayout(list_layout)
        right_layout.addWidget(list_group, 1)
        
        right_widget.setLayout(right_layout)
        splitter.addWidget(right_widget)
        
        # 设置分割比例
        splitter.setStretchFactor(0, 2)
        splitter.setStretchFactor(1, 1)
        splitter.setSizes([800, 400])
        
        main_layout.addWidget(splitter, 1)
    
    def toggle_camera(self):
        """切换摄像头状态"""
        if self.camera_thread and self.camera_thread.running:
            # 停止摄像头
            self.camera_thread.stop_camera()
            self.camera_thread.wait()
            self.camera_btn.setText('📷 启动摄像头')
            self.recognize_btn.setEnabled(self.current_image is not None)
            self.status_label.setText('状态: 摄像头已停止')
            self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #fff3e0; border-radius: 5px; color: #e65100;")
        else:
            # 启动摄像头
            self.camera_thread = CameraThread(recognition_interval=self.recognition_interval)
            self.camera_thread.frame_ready.connect(self.update_camera_frame)
            self.camera_thread.frame_for_recognition.connect(self.auto_recognize_plate)
            if self.camera_thread.start_camera():
                self.camera_btn.setText('⏹ 停止摄像头')
                self.recognize_btn.setEnabled(True)
                self.status_label.setText('状态: 摄像头运行中，每10帧自动识别')
                self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px; color: #2e7d32;")
            else:
                QMessageBox.warning(self, '错误', '无法打开摄像头')
    
    def update_camera_frame(self, frame):
        """更新摄像头画面"""
        self.current_image = frame.copy()
        self.display_image(frame)
    
    def auto_recognize_plate(self, frame):
        """自动识别车牌（每10帧调用一次）"""
        if not self.auto_recognize_enabled:
            return
        
        if self.recognition_model is None:
            return
        
        # 在后台线程中执行识别，避免阻塞UI
        try:
            # 使用当前帧进行识别
            result = self.recognition_model.get(frame)
            
            # 处理识别结果
            plate_number = ''
            confidence = 0.0
            
            if isinstance(result, str):
                plate_number = result
            elif result and len(result) > 0:
                if isinstance(result[0], (list, tuple)) and len(result[0]) >= 2:
                    plate_number = result[0][0]
                    confidence = result[0][1]
                elif isinstance(result[0], (list, tuple)) and len(result[0]) >= 1:
                    plate_number = result[0][0]
                    confidence = 1.0
                else:
                    plate_number = str(result[0])
                    confidence = 1.0
            
            # 更新显示（在主线程中）
            if plate_number and plate_number != '未检测到车牌':
                self.result_label.setText(f'识别结果: {plate_number}')
                # 比对数据库（静默模式，不弹窗）
                self.check_plate_in_database_silent(plate_number)
            else:
                self.result_label.setText('识别结果: 未检测到车牌')
                self.status_label.setText('状态: 摄像头运行中，自动识别中...')
                self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px; color: #2e7d32;")
        except Exception as e:
            print(f"自动识别错误: {e}")
            # 不显示错误对话框，避免频繁弹窗
    
    def upload_image(self):
        """上传图片"""
        try:
            file_path, _ = QFileDialog.getOpenFileName(
                self, '选择图片', '', '图片文件 (*.jpg *.jpeg *.png *.bmp *.JPG *.JPEG *.PNG *.BMP)'
            )
            if not file_path:
                return  # 用户取消了选择
            
            if not os.path.exists(file_path):
                QMessageBox.warning(self, '错误', f'文件不存在: {file_path}')
                return
            
            # 使用多种方法读取图片，解决中文路径和格式问题
            image = None
            
            # 方法1: 先读取为字节，然后解码（解决中文路径问题，推荐方法）
            try:
                with open(file_path, 'rb') as f:
                    file_bytes = f.read()
                    nparr = np.frombuffer(file_bytes, np.uint8)
                    image = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                if image is not None and image.size > 0:
                    print(f"使用方法1成功读取图片: {file_path}")
            except Exception as e1:
                print(f"方法1读取失败: {e1}")
            
            # 方法2: 使用PIL读取然后转换（备用方法，支持更多格式）
            if image is None or image.size == 0:
                try:
                    from PIL import Image as PILImage
                    pil_image = PILImage.open(file_path)
                    # 转换为RGB（如果是RGBA或其他格式）
                    if pil_image.mode != 'RGB':
                        pil_image = pil_image.convert('RGB')
                    # 转换为numpy数组
                    img_array = np.array(pil_image)
                    # PIL是RGB，OpenCV需要BGR
                    image = cv2.cvtColor(img_array, cv2.COLOR_RGB2BGR)
                    print(f"使用方法2成功读取图片: {file_path}")
                except Exception as e2:
                    print(f"方法2读取失败: {e2}")
            
            # 方法3: 直接使用cv2.imread（最后尝试）
            if image is None or image.size == 0:
                try:
                    image = cv2.imread(file_path)
                    if image is not None and image.size > 0:
                        print(f"使用方法3成功读取图片: {file_path}")
                except Exception as e3:
                    print(f"方法3读取失败: {e3}")
            
            if image is not None and image.size > 0:
                self.current_image = image.copy()
                self.display_image(self.current_image)
                self.recognize_btn.setEnabled(True)
                # 停止摄像头（如果正在运行）
                if self.camera_thread and self.camera_thread.running:
                    self.toggle_camera()
                self.status_label.setText(f'状态: 已加载图片 - {os.path.basename(file_path)}')
                self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #e3f2fd; border-radius: 5px; color: #1976d2;")
            else:
                QMessageBox.warning(
                    self, 
                    '错误', 
                    '无法读取图片文件。\n可能的原因：\n1. 文件格式不支持\n2. 文件已损坏\n3. 文件路径包含特殊字符\n\n请尝试使用其他图片文件。'
                )
        except Exception as e:
            QMessageBox.critical(self, '错误', f'上传图片失败: {str(e)}')
            print(f"上传图片错误: {e}")
            import traceback
            traceback.print_exc()
    
    def display_image(self, image):
        """显示图片"""
        try:
            # 转换颜色空间
            rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            h, w, ch = rgb_image.shape
            bytes_per_line = ch * w
            qt_image = QImage(rgb_image.data, w, h, bytes_per_line, QImage.Format_RGB888)
            
            # 获取标签的实际可用大小
            label_size = self.image_label.size()
            if label_size.width() <= 0 or label_size.height() <= 0:
                # 如果标签大小无效，使用默认大小
                label_size = self.image_label.minimumSize()
            
            # 计算缩放比例，保持宽高比
            pixmap = QPixmap.fromImage(qt_image)
            scaled_pixmap = pixmap.scaled(
                label_size.width() - 10,  # 留出边距
                label_size.height() - 10,
                Qt.KeepAspectRatio, 
                Qt.SmoothTransformation
            )
            self.image_label.setPixmap(scaled_pixmap)
            self.image_label.setText('')  # 清除文本提示
        except Exception as e:
            print(f"显示图片错误: {e}")
            QMessageBox.warning(self, '错误', f'显示图片失败: {str(e)}')
    
    def recognize_plate(self):
        """识别车牌"""
        if self.current_image is None:
            QMessageBox.warning(self, '警告', '请先选择图片或启动摄像头')
            return
        
        if self.recognition_model is None:
            QMessageBox.warning(self, '警告', '模型未加载，无法识别')
            return
        
        try:
            self.status_label.setText('状态: 正在识别...')
            self.status_label.setStyleSheet("font-size: 14px; padding: 5px; color: blue;")
            QApplication.processEvents()
            
            # 进行识别
            result = self.recognition_model.get(self.current_image)
            
            # 处理识别结果
            plate_number = ''
            confidence = 0.0
            
            if isinstance(result, str):
                plate_number = result
            elif result and len(result) > 0:
                if isinstance(result[0], (list, tuple)) and len(result[0]) >= 2:
                    plate_number = result[0][0]
                    confidence = result[0][1]
                elif isinstance(result[0], (list, tuple)) and len(result[0]) >= 1:
                    plate_number = result[0][0]
                    confidence = 1.0
                else:
                    plate_number = str(result[0])
                    confidence = 1.0
            
            # 更新显示
            if plate_number and plate_number != '未检测到车牌':
                self.result_label.setText(f'识别结果: {plate_number}')
                
                # 比对数据库（静默模式，不弹窗）
                self.check_plate_in_database_silent(plate_number)
            else:
                self.result_label.setText('识别结果: 未检测到车牌')
                self.status_label.setText('状态: 识别失败')
                self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #ffebee; border-radius: 5px; color: #c62828;")
                
        except Exception as e:
            QMessageBox.critical(self, '错误', f'识别失败: {str(e)}')
            self.status_label.setText(f'状态: 识别错误 - {str(e)}')
            self.status_label.setStyleSheet("font-size: 14px; padding: 5px; color: red;")
    
    def check_plate_in_database(self, plate_number, silent=False):
        """检查车牌是否在数据库中"""
        try:
            # 查询数据库
            registered_plate = self.db_session.query(RegisteredPlate).filter_by(
                plate_number=plate_number
            ).first()
            
            if registered_plate:
                # 车牌已注册
                self.result_label.setText(f'识别结果: {plate_number}')
                self.status_label.setText(f'状态: ✓ 车牌号请通过')
                self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #e8f5e9; border-radius: 5px; color: #2e7d32; font-weight: bold;")
                
                # 显示车主信息
                owner_info = f"车主: {registered_plate.owner_name}" if registered_plate.owner_name else ""
                if not silent:
                    QMessageBox.information(
                        self, 
                        '比对通过', 
                        f'车牌号: {plate_number}\n{owner_info}\n状态: 已注册，请通过！'
                    )
            else:
                # 车牌未注册
                self.result_label.setText(f'识别结果: {plate_number}')
                self.status_label.setText('状态: ✗ 车牌未注册')
                self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #fff3e0; border-radius: 5px; color: #e65100;")
                
                if not silent:
                    QMessageBox.warning(
                        self, 
                        '比对失败', 
                        f'车牌号: {plate_number}\n状态: 未注册，无法通过！'
                    )
                
        except Exception as e:
            if not silent:
                QMessageBox.critical(self, '错误', f'数据库查询失败: {str(e)}')
            self.status_label.setText(f'状态: 数据库错误')
            self.status_label.setStyleSheet("font-size: 14px; padding: 10px; background-color: #ffebee; border-radius: 5px; color: #c62828;")
    
    def check_plate_in_database_silent(self, plate_number):
        """静默检查车牌是否在数据库中（不弹窗）"""
        self.check_plate_in_database(plate_number, silent=True)
    
    def register_plate(self):
        """注册车牌"""
        plate_number = self.plate_input.text().strip()
        if not plate_number:
            QMessageBox.warning(self, '警告', '请输入车牌号')
            return
        
        try:
            # 检查是否已存在
            existing = self.db_session.query(RegisteredPlate).filter_by(
                plate_number=plate_number
            ).first()
            
            if existing:
                QMessageBox.warning(self, '警告', f'车牌号 {plate_number} 已注册')
                return
            
            # 创建新记录
            new_plate = RegisteredPlate(
                plate_number=plate_number,
                owner_name=self.owner_input.text().strip() or None,
                notes=self.notes_input.text().strip() or None,
                register_time=datetime.now()
            )
            
            self.db_session.add(new_plate)
            self.db_session.commit()
            
            QMessageBox.information(self, '成功', f'车牌号 {plate_number} 注册成功')
            
            # 清空输入框
            self.plate_input.clear()
            self.owner_input.clear()
            self.notes_input.clear()
            
            # 刷新列表
            self.load_registered_plates()
            
        except Exception as e:
            self.db_session.rollback()
            QMessageBox.critical(self, '错误', f'注册失败: {str(e)}')
    
    def delete_plate(self):
        """删除选中的车牌"""
        current_row = self.plate_table.currentRow()
        if current_row < 0:
            QMessageBox.warning(self, '警告', '请选择要删除的车牌')
            return
        
        plate_id = int(self.plate_table.item(current_row, 0).text())
        plate_number = self.plate_table.item(current_row, 1).text()
        
        reply = QMessageBox.question(
            self, 
            '确认删除', 
            f'确定要删除车牌号 {plate_number} 吗？',
            QMessageBox.Yes | QMessageBox.No
        )
        
        if reply == QMessageBox.Yes:
            try:
                plate = self.db_session.query(RegisteredPlate).filter_by(id=plate_id).first()
                if plate:
                    self.db_session.delete(plate)
                    self.db_session.commit()
                    QMessageBox.information(self, '成功', '删除成功')
                    self.load_registered_plates()
            except Exception as e:
                self.db_session.rollback()
                QMessageBox.critical(self, '错误', f'删除失败: {str(e)}')
    
    def load_registered_plates(self):
        """加载已注册车牌列表"""
        try:
            plates = self.db_session.query(RegisteredPlate).order_by(
                RegisteredPlate.register_time.desc()
            ).all()
            
            self.plate_table.setRowCount(len(plates))
            for row, plate in enumerate(plates):
                self.plate_table.setItem(row, 0, QTableWidgetItem(str(plate.id)))
                self.plate_table.setItem(row, 1, QTableWidgetItem(plate.plate_number))
                self.plate_table.setItem(row, 2, QTableWidgetItem(plate.owner_name or ''))
                self.plate_table.setItem(row, 3, QTableWidgetItem(
                    plate.register_time.strftime('%Y-%m-%d %H:%M:%S') if plate.register_time else ''
                ))
            
            self.plate_table.resizeColumnsToContents()
            
        except Exception as e:
            QMessageBox.critical(self, '错误', f'加载列表失败: {str(e)}')
    
    def closeEvent(self, event):
        """关闭事件"""
        if self.camera_thread and self.camera_thread.running:
            self.camera_thread.stop_camera()
            self.camera_thread.wait()
        self.db_session.close()
        event.accept()


def main():
    """主函数"""
    app = QApplication(sys.argv)
    
    # 设置应用程序样式
    app.setStyle('Fusion')
    
    window = CarPlateRecognitionApp()
    window.show()
    
    sys.exit(app.exec_())


if __name__ == '__main__':
    main()
