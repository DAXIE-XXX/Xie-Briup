import QtQuick

Item {
    Image {
        id: mapImg
        source: "images/RoadMap.jpg"
        visible: false
    }

    // 车辆图片
    Image {
        id: carImg
        source: "images/car.png"
        visible: false
    }

    Canvas {
        id: cav
        anchors.fill: parent

        onPaint: {
            var ctx = getContext('2d')
            // 清除现有界面的内容
            ctx.clearRect(0, 0, width, height)

            var dx, dy, dw, dh
            var scale = Math.min(width / mapImg.width, height / mapImg.height)
            dw = mapImg.width * scale
            dh = mapImg.height * scale
            dx = (width - dw) / 2
            dy = (height - dh) / 2
            ctx.drawImage(mapImg, dx, dy, dw, dh)

            // ========== 获取道路和红绿灯数据 ==========
            var rl = dc.getRoadList();
            var l = rl.length
            var rx, ry, rw, rh, rc

            // ========== 绘制道路 ==========
            for(var i = 0; i < l; i++)
            {
                rx = rl[i]['x']
                ry = rl[i]['y']
                rw = rl[i]['w']
                rh = rl[i]['h']
                rc = rl[i]['color']

                var scaledX = dx + rx * scale
                var scaledY = dy + ry * scale
                var scaledW = rw * scale
                var scaledH = rh * scale

                ctx.beginPath()
                ctx.rect(scaledX, scaledY, scaledW, scaledH)
                ctx.fillStyle = (rc === 0 ? "green" : rc === 1 ? "yellow" : "red")
                ctx.fill()
                ctx.closePath()
            }

            // ========== 绘制红绿灯 ==========
            var ll = dc.getLightList();
            l = ll.length
            var roadX, roadY, roadW, roadH, roadDir, lightColor
            for(i = 0; i < l; i++){
                roadX = ll[i]['roadX']
                roadY = ll[i]['roadY']
                roadW = ll[i]['roadW']
                roadH = ll[i]['roadH']
                roadDir = ll[i]['roadDir']
                lightColor = ll[i]['color']

                var lightX, lightY, lightW, lightH
                switch (roadDir){
                case 0: // 左
                    lightW = 4
                    lightH = 28
                    lightX = roadX - lightW - 3
                    lightY = roadY - 9
                    break
                case 1: // 右
                    lightW = 4
                    lightH = 28
                    lightX = roadX + roadW + 3
                    lightY = roadY - 9
                    break
                case 2: // 上
                    lightW = 28
                    lightH = 4
                    lightX = roadX - 9
                    lightY = roadY - lightH - 3
                    break
                case 3: // 下
                    lightW = 28
                    lightH = 4
                    lightX = roadX - 9
                    lightY = roadY + roadH + 3
                    break
                default:
                    continue
                }

                if(lightColor === -1)
                    continue

                var scaledLightX = dx + lightX * scale
                var scaledLightY = dy + lightY * scale
                var scaledLightW = lightW * scale
                var scaledLightH = lightH * scale

                ctx.beginPath()
                ctx.rect(scaledLightX, scaledLightY, scaledLightW, scaledLightH)
                ctx.fillStyle = (lightColor === 0 ? "green" : lightColor === 1 ? "yellow" : "red")
                ctx.fill()
                ctx.closePath()
            }

            // ========== 绘制车辆 + 固定序号（使用 car_id）==========
            var carList = dc.getCarList();
            var roadList = dc.getRoadList();
            var carScale = 0.10;   // 小车放大比例

            for(var idx = 0; idx < carList.length; idx++) {
                var car = carList[idx];
                var roadId = car.road_id;
                var roadPos = car.road_pos;
                var cx, cy;

                // 计算车辆中心点坐标（逻辑坐标）
                if(roadId < 0 || roadPos < 0) {
                    // 停车区车辆
                    cx = mapImg.width * 0.752;
                    cy = mapImg.height * 0.78 + idx * 20;
                } else {
                    var road = roadList[roadId];
                    if(!road) continue;
                    if (roadId === 0) {
                        cx = road.x + road.w * (100 - roadPos) / 100;
                        cy = road.y + road.h / 2;
                    } else if (roadId === 13) {
                        cx = road.x + road.w / 2;
                        cy = road.y + road.h * (100 - roadPos) / 100;
                    } else if (roadId === 9) {
                        cx = road.x + road.w / 2;
                        cy = road.y + road.h * (100 - roadPos) / 100;
                    } else {
                        if(road.dir === 0 || road.dir === 1) {
                            cx = road.x + road.w * roadPos / 100;
                            cy = road.y + road.h / 2;
                        } else {
                            cx = road.x + road.w / 2;
                            cy = road.y + road.h * roadPos / 100;
                        }
                    }
                }

                var realX = dx + cx * dw / mapImg.width;
                var realY = dy + cy * dh / mapImg.height;
                var carW = carImg.width * carScale;
                var carH = carImg.height * carScale;
                var imgX = realX - carW/2;
                var imgY = realY - carH/2;

                ctx.drawImage(carImg, imgX, imgY, carW, carH);

                // 获取固定序号：优先使用 car.car_id，若没有则使用 car.id，最后使用索引（不推荐）
                var seq = (car.car_id !== undefined) ? car.car_id : (car.id !== undefined ? car.id : idx);
                // 限制只显示 0,1,2（假如只有三辆车）
                if(seq > 2) seq = idx; // fallback

                ctx.font = "bold " + Math.max(12, cav.height * 0.03) + "px sans-serif";
                ctx.textAlign = "center";
                ctx.textBaseline = "middle";

                var textX = realX;
                var textY = realY - carH/2 - 5;
                var textMetrics = ctx.measureText(seq);
                var textWidth = textMetrics.width;
                var textHeight = parseInt(ctx.font);
                var bgPadding = 4;
                var bgX = textX - textWidth/2 - bgPadding;
                var bgY = textY - textHeight/2 - bgPadding;
                var bgW = textWidth + bgPadding*2;
                var bgH = textHeight + bgPadding*2;

                ctx.fillStyle = "rgba(255,255,255,0.7)";
                ctx.fillRect(bgX, bgY, bgW, bgH);
                ctx.fillStyle = "black";
                ctx.fillText(seq, textX, textY);
            }
        }
    }

    // 连接数据变化信号
    Component.onCompleted: {
        dc.dataChanged.connect(dataChangedSlot)
        // 强制刷新一次
        cav.requestPaint()
    }

    function dataChangedSlot(){
        cav.requestPaint()
    }

    // 定时刷新动画效果（用于双闪闪烁和车辆移动）
    Timer {
        running: true
        repeat: true
        interval: 50  // 50ms 刷新一次，保证动画流畅
        onTriggered: {
            cav.requestPaint()
        }
    }
}