import onnx

# 加载原始模型
model = onnx.load("./yolo_best.onnx")

# 检查并修改IR版本
if model.ir_version > 9:
    model.ir_version = 9  # 设置为支持的最高版本

# 保存修改后的模型
onnx.save(model, "./yolo_best_low_ir_version.onnx")