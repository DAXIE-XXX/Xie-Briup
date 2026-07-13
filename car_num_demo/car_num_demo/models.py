from flask_sqlalchemy import SQLAlchemy
db=SQLAlchemy()

#车牌结果类
class car_num_res(db.Model):
    id=db.Column(db.Integer,primary_key=True,autoincrement=True)
    car_number=db.Column(db.String(50))
    times=db.Column(db.DateTime)
    image_path=db.Column(db.String(255))

#车牌模型存储表（保留兼容性）
class car_num_model_info(db.Model):
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    times = db.Column(db.DateTime)
    yolo_path = db.Column(db.String(255))
    ocr_path = db.Column(db.String(255))
    conf=db.Column(db.String(255))
    iou=db.Column(db.String(255))
    ocr_shape =db.Column(db.String(255))
    ocr_dict_path =db.Column(db.String(255))
    padding = db.Column(db.String(255))

#通用模型信息表（支持多种模型类型）
class model_info(db.Model):
    __tablename__ = 'model_info'
    id = db.Column(db.Integer, primary_key=True, autoincrement=True)
    model_type = db.Column(db.String(50), nullable=False)  # building_yolo, car_yolo_ocr, handwriting_cnn
    model_name = db.Column(db.String(255))  # 模型名称
    model_dir = db.Column(db.String(500), nullable=False)  # 模型目录路径
    upload_time = db.Column(db.DateTime, nullable=False)  # 上传时间
    
    # 通用配置字段（JSON格式存储，灵活支持不同模型类型的配置）
    config_json = db.Column(db.Text)  # 存储完整的配置JSON
    
    # 车牌识别模型专用字段（保留以便查询）
    yolo_path = db.Column(db.String(500))
    ocr_path = db.Column(db.String(500))
    ocr_dict_path = db.Column(db.String(500))
    conf = db.Column(db.String(50))
    iou = db.Column(db.String(50))
    ocr_shape = db.Column(db.String(100))
    padding = db.Column(db.String(50))
    
    # 建筑物识别模型专用字段
    building_yolo_path = db.Column(db.String(500))
    building_conf = db.Column(db.String(50))
    building_iou = db.Column(db.String(50))
    
    # 手写识别模型专用字段
    cnn_path = db.Column(db.String(500))
    input_shape = db.Column(db.String(100))
    num_classes = db.Column(db.String(50))
    
    def to_dict(self):
        """转换为字典格式"""
        import json
        result = {
            'id': self.id,
            'model_type': self.model_type,
            'model_name': self.model_name,
            'model_dir': self.model_dir,
            'upload_time': self.upload_time.strftime('%Y-%m-%d %H:%M:%S') if self.upload_time else None
        }
        if self.config_json:
            result['config'] = json.loads(self.config_json)
        return result