import QtQuick
import Qt.labs.platform

Window {
    width: 640
    height: 480
    visible: true
    property int id: 0
    title: qsTr("模拟车辆0")

    Menu {
        id: rightBtnMenu
        MenuItem {
            text: qsTr("解锁车辆")
            onTriggered: sc.setCarLock(false)
        }

        MenuItem {
            text: qsTr("锁定车辆")
            onTriggered: sc.setCarLock(true)
        }

        MenuItem {
            text: qsTr("启动车辆")
            onTriggered: sc.setCarRun(true)
        }

        MenuItem {
            text: qsTr("停止车辆")
            onTriggered: sc.setCarRun(false)
        }

        MenuItem {
            text: qsTr("打开大灯")
            onTriggered: dc.setCarLightSw(true)
        }
        MenuItem {
            text: qsTr("关闭大灯")
            onTriggered: dc.setCarLightSw(false)
        }
        MenuItem {
            text: qsTr("打开双闪")
            onTriggered: dc.setCarAlarmSw(true)
        }
        MenuItem {
            text: qsTr("关闭双闪")
            onTriggered: dc.setCarAlarmSw(false)
        }
    }


    Image {
        id: carImg
        visible: false
        source: "images/car.png"
    }

    Timer {
        id: alarmLightTimer
        running: true
        repeat: true
        interval: 500
        onTriggered: {
            if(!dc.getCarAlarmSw())
                return
            cav.alarmNowSt = !cav.alarmNowSt
            cav.requestPaint()
        }
    }

    Canvas {
        id: cav
        anchors.fill: parent
        property bool alarmNowSt: true
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onClicked: (mouse)=>{
                if(checkLightArea(mouseX, mouseY)){
                    dc.setCarLightSw(!dc.getCarLightSw())
                }
                if(checkAlarmArea(mouseX, mouseY)){
                    dc.setCarAlarmSw(!dc.getCarAlarmSw())
                }
                if(mouse.button === Qt.RightButton)
                    rightBtnMenu.open()
            }
        }

        onPaint: {
            var x, y, w, h
            if(width * carImg.height > height * carImg.width){
                //比较宽
                h = cav.height
                w = carImg.width * h / carImg.height
                x = (cav.width - w)/2
                y = 0
            }
            else {
                w = cav.width
                h = carImg.height * w / carImg.width
                x = 0
                y = (cav.height - h)/2
            }
            var ctx = getContext("2d")
            ctx.clearRect(0,0,cav.width,cav.height)
            ctx.drawImage(carImg, x, y, w, h)

            if(dc.getCarLightSw()){
                ctx.fillStyle = "yellow"
                var cl = dc.getCarLightList()
                var l = cl.length
                for(var i = 0; i < l; i++)
                {
                    ctx.beginPath()
                    var rx = cl[i]['x']
                    var ry = cl[i]['y']
                    var rw = cl[i]['w']
                    var rh = cl[i]['h']
                    ctx.beginPath()
                    ctx.arc(x + rx * w / carImg.width + (rw * w / carImg.width/2),
                            y + ry * h / carImg.height + (rh * h / carImg.height/2),
                            (rw * w / carImg.width + rh * h / carImg.height) / 4 , 0, Math.PI * 2, false);
                    ctx.closePath()
                    ctx.fill()
                }
            }
            if(dc.getCarAlarmSw() && alarmNowSt){
                ctx.fillStyle = "yellow"
                var cal = dc.getCarAlarmLightList()
                var l = cal.length
                for(var i = 0; i < l; i++)
                {
                    ctx.beginPath()
                    var rx = cal[i]['x']
                    var ry = cal[i]['y']
                    var rw = cal[i]['w']
                    var rh = cal[i]['h']
                    ctx.fillRect(x + rx * w / carImg.width, y + ry * h / carImg.height,
                                 rw * w / carImg.width, rh * h / carImg.height);
                }
            }
        }
    }
    //判断鼠标是否在灯范围内
    function checkLightArea(mx, my){
        //计算图片实际在界面上的尺寸，图片尺寸268 * 220
        var x, y
        var w, h
        var iw = carImg.width, ih = carImg.height
        if(cav.width * ih > cav.height * iw){
            //界面比较宽
            h = cav.height
            w = iw * h / ih
            x = (cav.width - w)/2
            y = 0
        }
        else{
            w = cav.width
            h = ih * w / iw
            y = (cav.height - h)/2
            x = 0
        }
        var ll = dc.getCarLightList();
        var l = ll.length
        var press = false
        for(var i = 0; i < l; i++)
        {
            var rx = ll[i]['x']
            var ry = ll[i]['y']
            var rw = ll[i]['w']
            var rh = ll[i]['h']
            var light_x = x + rx*w/iw
            var right_x = x + (rx+rw)*w/iw;
            var up_y = y + ry*h/ih
            var down_y = y + (ry+rh)*h/ih
            if((mx >= light_x && mx <= right_x) &&
                (my >= up_y && my <= down_y)){
                press = true
            }
        }
        return press
    }
    //判断鼠标是否在双闪范围内
    function checkAlarmArea(mx, my){
        //计算图片实际在界面上的尺寸，图片尺寸268 * 220
        var x, y
        var w, h
        var iw = carImg.width, ih = carImg.height
        if(cav.width * ih > cav.height * iw){
            //界面比较宽
            h = cav.height
            w = iw * h / ih
            x = (cav.width - w)/2
            y = 0
        }
        else{
            w = cav.width
            h = ih * w / iw
            y = (cav.height - h)/2
            x = 0
        }
        var al = dc.getCarAlarmLightList();
        var l = al.length
        var press = false
        for(var i = 0; i < l; i++)
        {
            var rx = al[i]['x']
            var ry = al[i]['y']
            var rw = al[i]['w']
            var rh = al[i]['h']
            var light_x = x + rx*w/iw
            var right_x = x + (rx+rw)*w/iw;
            var up_y = y + ry*h/ih
            var down_y = y + (ry+rh)*h/ih
            if((mx >= light_x && mx <= right_x) &&
                (my >= up_y && my <= down_y)){
                press = true
            }
        }
        return press
    }
    Component.onCompleted: {
        dc.carLightSwChanged.connect(cav.requestPaint)
        dc.carAlarmSwChanged.connect(cav.requestPaint)
        id = sc.getCarId();
        title = '模拟车辆' + String(id)
        sc.lockStateChanged.connect(lockStateChangedSlot)
        sc.runStateChanged.connect(runStateChangedSlot)
    }
    function lockStateChangedSlot(sw){
        title = '模拟车辆' + String(id) + '-' + (sw ? "已锁定" : "已解锁")
    }
    function runStateChangedSlot(sw){
        title = '模拟车辆' + String(id) + '-' + (sw ? "已启动" : "已停止")
    }
}




