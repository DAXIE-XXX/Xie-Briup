import math
import time

import cv2
import onnxruntime as ort
import numpy as np
from PIL import Image, ImageDraw, ImageFont

class Yolo:
    def __init__(self, yolo_path, confidence_thres=0.35, iou_thres=0.5):
        # 初始化类属性
        self.confidence_thres = confidence_thres  # 置信度阈值
        self.iou_thres = iou_thres  # IOU阈值
        # 判断是使用GPU还是CPU
        self.__providers = [
            ('CUDAExecutionProvider', {'device_id': 0}),  # 可以选择GPU设备ID
            'CPUExecutionProvider'  # 也可以设置CPU作为备选
        ]
        # 初始化检测模型
        self.__detect_session, self.__model_inputs, self.__input_width, self.__input_height = self.__init_detect_model(
            yolo_path)

    def __init_detect_model(self, model_path):
        # 使用ONNX模型文件创建一个推理会话，并指定执行提供者
        session = ort.InferenceSession(model_path, providers=self.__providers)
        # 获取模型的输入信息
        model_inputs = session.get_inputs()
        # 获取输入的形状，用于后续使用
        input_shape = model_inputs[0].shape
        # 从输入形状中提取输入宽度
        input_width = input_shape[2]
        # 从输入形状中提取输入高度
        input_height = input_shape[3]
        # 返回会话、模型输入信息、输入宽度和输入高度
        return session, model_inputs, input_width, input_height

    def __calculate_iou(self, box, other_boxes):
        """
        计算给定边界框与一组其他边界框之间的交并比（IoU）。

        参数：
        - box: 单个边界框，格式为 [x1, y1, width, height]。
        - other_boxes: 其他边界框的数组，每个边界框的格式也为 [x1, y1, width, height]。

        返回值：
        - iou: 一个数组，包含给定边界框与每个其他边界框的IoU值。
        """
        # 计算交集的左上角坐标
        x1 = np.maximum(box[0], np.array(other_boxes)[:, 0])
        y1 = np.maximum(box[1], np.array(other_boxes)[:, 1])
        # 计算交集的右下角坐标
        x2 = np.minimum(box[0] + box[2], np.array(other_boxes)[:, 0] + np.array(other_boxes)[:, 2])
        y2 = np.minimum(box[1] + box[3], np.array(other_boxes)[:, 1] + np.array(other_boxes)[:, 3])
        # 计算交集区域的面积
        intersection_area = np.maximum(0, x2 - x1) * np.maximum(0, y2 - y1)
        # 计算给定边界框的面积
        box_area = box[2] * box[3]
        other_boxes_area = np.array(other_boxes)[:, 2] * np.array(other_boxes)[:, 3]
        # 计算IoU值
        iou = intersection_area / (box_area + other_boxes_area - intersection_area)
        return iou

    def __custom_NMSBoxes(self, boxes, scores, confidence_threshold, iou_threshold):
        # 如果没有边界框，则直接返回空列表
        if len(boxes) == 0:
            return []
        # 将得分和边界框转换为NumPy数组
        scores = np.array(scores)
        boxes = np.array(boxes)
        # 根据置信度阈值过滤边界框
        mask = scores > confidence_threshold
        filtered_boxes = boxes[mask]
        filtered_scores = scores[mask]
        # 如果过滤后没有边界框，则返回空列表
        if len(filtered_boxes) == 0:
            return []
        # 根据置信度得分对边界框进行排序
        sorted_indices = np.argsort(filtered_scores)[::-1]
        # 初始化一个空列表来存储选择的边界框索引
        indices = []
        # 当还有未处理的边界框时，循环继续
        while len(sorted_indices) > 0:
            # 选择得分最高的边界框索引
            current_index = sorted_indices[0]
            indices.append(current_index)
            # 如果只剩一个边界框，则结束循环
            if len(sorted_indices) == 1:
                break
            # 获取当前边界框和其他边界框
            current_box = filtered_boxes[current_index]
            other_boxes = filtered_boxes[sorted_indices[1:]]
            # 计算当前边界框与其他边界框的IoU
            iou = self.__calculate_iou(current_box, other_boxes)
            # 找到IoU低于阈值的边界框，即与当前边界框不重叠的边界框
            non_overlapping_indices = np.where(iou <= iou_threshold)[0]
            # 更新sorted_indices以仅包含不重叠的边界框
            sorted_indices = sorted_indices[non_overlapping_indices + 1]
        # 返回选择的边界框索引
        return indices

    def __preprocess(self, img, input_width, input_height):
        """
        在执行推理之前预处理输入图像。

        返回:
            image_data: 为推理准备好的预处理后的图像数据。
        """
        # 获取输入图像的高度和宽度
        img_height, img_width = img.shape[:2]
        # 将图像颜色空间从BGR转换为RGB
        img = cv2.cvtColor(img, cv2.COLOR_BGR2RGB)
        # 将图像大小调整为匹配输入形状
        img = cv2.resize(img, (input_width, input_height))
        # 通过除以255.0来归一化图像数据
        image_data = np.array(img) / 255.0
        # 转置图像，使通道维度为第一维
        image_data = np.transpose(image_data, (2, 0, 1))  # 通道首
        # 扩展图像数据的维度以匹配预期的输入形状
        image_data = np.expand_dims(image_data, axis=0).astype(np.float32)
        # 返回预处理后的图像数据
        return image_data, img_height, img_width

    def __postprocess(self, output, input_width, input_height, img_width, img_height):
        """
            对模型输出进行后处理，提取边界框、得分和类别ID。

            参数:
                input_image (numpy.ndarray): 输入图像。
                output (numpy.ndarray): 模型的输出。
                input_width (int): 模型输入宽度。
                input_height (int): 模型输入高度。
                img_width (int): 原始图像宽度。
                img_height (int): 原始图像高度。

            返回:
                input_image(numpy.ndarray): 绘制了检测结果的输入图像。
                ocr_list(list): 存储了ocr识别的结果
            """
        # 转置和压缩输出以匹配预期的形状
        outputs = np.transpose(np.squeeze(output[0]))
        # 获取输出数组的行数
        rows = outputs.shape[0]
        # 用于存储检测的边界框、得分和类别ID的列表
        boxes = []
        scores = []
        class_ids = []
        # 计算边界框坐标的缩放因子
        x_factor = img_width / input_width
        y_factor = img_height / input_height
        # 遍历输出数组的每一行
        for i in range(rows):
            # 从当前行提取类别得分
            classes_scores = outputs[i][4:]
            # 找到类别得分中的最大得分
            max_score = np.amax(classes_scores)
            # 如果最大得分高于置信度阈值
            if max_score >= self.confidence_thres:
                # 获取得分最高的类别ID
                class_id = np.argmax(classes_scores)
                # 从当前行提取边界框坐标
                x, y, w, h = outputs[i][0], outputs[i][1], outputs[i][2], outputs[i][3]
                # 计算边界框的缩放坐标
                #左上点坐标和宽高
                left = int((x - w / 2) * x_factor)
                top = int((y - h / 2) * y_factor)
                width = int(w * x_factor)
                height = int(h * y_factor)
                # 将类别ID、得分和框坐标添加到各自的列表中
                class_ids.append(class_id)
                scores.append(max_score)
                boxes.append([left, top, width, height])
        # 应用非最大抑制过滤重叠的边界框
        indices = self.__custom_NMSBoxes(boxes, scores, self.confidence_thres, self.iou_thres)
        result = []
        for i in indices:
            temp = []
            box = boxes[i]
            score = scores[i]
            class_id = class_ids[i]
            temp.append(box)
            temp.append(score)
            temp.append(class_id)
            result.append(temp)

        return result

    def detect_object(self, image):
        """
            进行推理的主函数

            参数:
                image (numpy.ndarray): 输入图像。
            返回:

            """
        # 如果输入的图像是PIL图像对象，将其转换为NumPy数组
        if isinstance(image, Image.Image):
            result_image = np.array(image)
        else:
            # 否则，直接使用输入的图像（假定已经是NumPy数组）
            result_image = image
        # 预处理图像数据，调整图像大小并可能进行归一化等操作
        img_data, img_height, img_width = self.__preprocess(result_image, self.__input_width, self.__input_height)
        # 使用预处理后的图像数据进行yolo推理
        outputs = self.__detect_session.run(None, {self.__model_inputs[0].name: img_data})
        # 对推理结果进行后处理，例如解码检测框，过滤低置信度的检测
        indices = self.__postprocess(outputs, self.__input_width, self.__input_height,
                                     img_width,
                                     img_height)
        # 返回处理后的图像，以及yolo列表
        return indices
class Ocr:
    def __init__(self, ocr_path, ocr_shape, ocr_dict_path, padding=True):
        self.__ocr_shape = ocr_shape
        self.__ocr_session = ort.InferenceSession(ocr_path)
        self.__postprocess_op = self.__process_pred(character_dict_path=ocr_dict_path, use_space_char=False)
        self.__padding = padding
        self.__input_name = self.__ocr_session.get_inputs()[0].name
        self.__output_name = self.__ocr_session.get_outputs()[0].name

    class __process_pred(object):
        def __init__(self, character_dict_path=None, use_space_char=False):
            self.character_str = ''

            with open(character_dict_path, 'rb') as fin:
                lines = fin.readlines()
                for line in lines:
                    line = line.decode('utf-8').strip('\n').strip('\r\n')
                    self.character_str += line
            if use_space_char:
                self.character_str += ' '
            dict_character = list(self.character_str)

            dict_character = self.add_special_char(dict_character)
            self.dict = {}
            for i, char in enumerate(dict_character):
                self.dict[char] = i
            self.character = dict_character

        def add_special_char(self, dict_character):
            dict_character = ['blank'] + dict_character
            return dict_character

        def decode(self, text_index, text_prob=None, is_remove_duplicate=False):
            result_list = []
            ignored_tokens = [0]
            batch_size = len(text_index)
            for batch_idx in range(batch_size):
                char_list = []
                conf_list = []
                for idx in range(len(text_index[batch_idx])):
                    if text_index[batch_idx][idx] in ignored_tokens:
                        continue
                    if is_remove_duplicate:
                        if idx > 0 and text_index[batch_idx][idx - 1] == text_index[batch_idx][idx]:
                            continue
                    char_list.append(self.character[int(text_index[batch_idx][idx])])
                    if text_prob is not None:
                        conf_list.append(text_prob[batch_idx][idx])
                    else:
                        conf_list.append(1)
                text = ''.join(char_list)
                result_list.append((text, np.mean(conf_list)))
            return result_list

        def __call__(self, preds, label=None):
            if not isinstance(preds, np.ndarray):
                preds = np.array(preds)
            preds_idx = preds.argmax(axis=2)
            preds_prob = preds.max(axis=2)
            text = self.decode(preds_idx, preds_prob, is_remove_duplicate=True)
            if label is None:
                return text
            label = self.decode(label)
            return text, label

    def resize_norm_img(self, img):
        imgC, imgH, imgW = self.__ocr_shape
        h = img.shape[0]
        w = img.shape[1]
        if not self.__padding:
            resized_image = cv2.resize(
                img, (imgW, imgH), interpolation=cv2.INTER_LINEAR)
            resized_w = imgW
        else:
            ratio = w / float(h)
            if math.ceil(imgH * ratio) > imgW:
                resized_w = imgW
            else:
                resized_w = int(math.ceil(imgH * ratio))
            resized_image = cv2.resize(img, (resized_w, imgH))
        resized_image = resized_image.astype('float32')
        if self.__ocr_shape[0] == 1:
            resized_image = resized_image / 255
            resized_image = resized_image[np.newaxis, :]
        else:
            resized_image = resized_image.transpose((2, 0, 1)) / 255
        resized_image -= 0.5
        resized_image /= 0.5
        padding_im = np.zeros((imgC, imgH, imgW), dtype=np.float32)
        padding_im[:, :, 0:resized_w] = resized_image
        return padding_im

    def get_ocr(self, img):

        # 加载图像并进行预处理
        image = self.resize_norm_img(img)
        image = np.expand_dims(image, axis=0).astype(np.float32)

        # 执行推理
        outputs = self.__ocr_session.run([self.__output_name], {self.__input_name: image})

        return self.__postprocess_op(outputs[0])
#测试示例
# #测试示例
# if __name__ == "__main__":
#     yolo = Yolo('./yolo_best.onnx')
#     img = cv2.imread('./test1.jpg')
#     detections = yolo.detect_object(img)
#     print(detections)
# if __name__ == "__main__":
#     ocr = Ocr(
#         ocr_path='./ocr_best.onnx',
#         ocr_shape=(3, 48, 320),
#         ocr_dict_path='./ppocr_keys_v1.txt',
#         padding=True
#     )
#     img = cv2.imread('./test1.jpg')
#     result = ocr.get_ocr(img)
#     print(result)

#作业
# 已完成  车牌位置检测输出坐标 置信度 类别
#         车牌号码输出
"""需要完成 ：创建一个新的类，
    三个参数： 车牌位置检测的模型  车牌文字识别模型  图片
    输出：车牌文字
    思考：如何定义这个类 让用户使用更方便"""
class car_num():
    def __init__(self,yolo,conf,iou,ocr,ocr_shape,ocr_dict_path,padding):
        self.yolo=yolo
        self.ocr=ocr
        self.conf=conf
        self.iou=iou
        self.ocr_shape=ocr_shape
        self.ocr_dict_path=ocr_dict_path
        self.padding=padding
        print(self.ocr_dict_path,self.ocr_shape)
    def get(self,img):
        yolo_model=Yolo(self.yolo,self.conf,self.iou)
        yolo_res=yolo_model.detect_object(img)
        #yolo_res
        try:
            print(yolo_res)
            box=yolo_res[0][0]
        except:
            return '未检测到车牌'
        #box 左上点坐标和宽高度
        x1 = int(box[0])
        y1 = int(box[1])
        x2 = int(box[0] + box[2])
        y2 = int(box[1] + box[3])
        #按车牌左上点坐标和右下点坐标切割只包含车牌的图片
        car_num_img = img[y1:y2, x1:x2]
        # cv2.rectangle(img,(x1,y1),(x2,y2),(0,255,0),3)
        # car_num_img=img[y1:y2,x1:x2]
        # cv2.imshow('img',car_num_img)
        # #能够长时间显示 加等待
        # cv2.waitKey(0)
        ocr_model=Ocr(
            ocr_path=self.ocr,
            ocr_shape=self.ocr_shape,
            ocr_dict_path=self.ocr_dict_path,
            padding=self.padding
        )
        ocr_res=ocr_model.get_ocr(car_num_img)
        return ocr_res

if __name__ == '__main__':
    yolo_path='./yolo_best.onnx'
    ocr_path='./ocr_best.onnx'
    conf=0.6
    iou=0.5
    ocr_shape = (3, 48, 320)
    ocr_dict_path ='./ppocr_keys_v1.txt'
    padding = True
    img_path='./test2.jpg'
    img=cv2.imread(img_path)
    img2=cv2.imread('./test3.jpg')
    car=car_num(
        yolo=yolo_path,
        ocr=ocr_path,
        conf=conf,
        iou=iou,
        ocr_shape=ocr_shape,
        ocr_dict_path=ocr_dict_path,
        padding=padding
    )
    res=car.get(img)
    # res1=car.get(img2)
    if isinstance(res,type('str')):
        print(res)
    else:
        print(res[0][1])


#练习  当前这个类  如何修改 能够只加载一次模型
  # 用户使用只需传图片
  # 运行  模型也要同步加载
  # 2.用户可以添加新的模型
  # 3.用户使用时仅1.服务器器需传图片
#
#
#
# 1.用户上传图片识别结果
# 2.用户可以修改模型
# 3.用户可以上传新的模型


