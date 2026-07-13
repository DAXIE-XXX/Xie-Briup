import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("模拟路口")

    Image {
        id: crossingImg
        visible: false
        source: "images/RoadSig.jpg"
    }

    Canvas {
        id: cav
        anchors.fill: parent
        onPaint: {
            var x, y, w, h
            if(width * crossingImg.height > height * crossingImg.width){
                h = cav.height
                w = crossingImg.width * h / crossingImg.height
                x = (cav.width - w)/2
                y = 0
            }
            else {
                w = cav.width
                h = crossingImg.height * w / crossingImg.width
                x = 0
                y = (cav.height - h)/2
            }
            var ctx = getContext("2d")
            ctx.clearRect(0,0,cav.width,cav.height)
            ctx.drawImage(crossingImg, x, y, w, h)

            var rl = dc.getLightList()
            var l = rl.length

            var iw = crossingImg.width
            var ih = crossingImg.height
            for(var i = 0; i < l; i++)
            {
                if(!rl[i]['show'])
                    continue
                var rx = rl[i]['x']
                var ry = rl[i]['y']
                var rw = rl[i]['w']
                var rh = rl[i]['h']
                var rc = rl[i]['color']
                var rd = rl[i]['dir']
                if(rd === -1)
                    continue
                // 颜色映射: 0=绿灯, 1=黄灯, 2=红灯, -1=灭灯(白色/灰色)
                if(rc === 0){
                    ctx.fillStyle = "green"
                } else if(rc === 1){
                    ctx.fillStyle = "yellow"
                } else if(rc === 2){
                    ctx.fillStyle = "red"
                } else {
                    ctx.fillStyle = "gray"
                }
                ctx.fillRect(x + rx * w / iw, y + ry * h / ih,
                         rw * w / iw, rh * h / ih)
            }
        }
    }

    Connections {
        target: sc
        function onModeChanged(mode) {
            console.log("模式变化信号收到，新模式:", mode)
            updateTitle(mode)
        }
        function onCrossingIdChanged(id) {
            console.log("路口ID变化信号收到，新ID:", id)
            updateTitle()
        }
    }

    Component.onCompleted: {
        dc.lightColorChanged.connect(lightColorChangedSlot)
        updateTitle()
        console.log("初始化完成")
    }

    function lightColorChangedSlot(){
        cav.requestPaint()
    }

    function getCrossingId() {
        if (sc.getCrossingId) {
            return sc.getCrossingId()
        }
        return 0
    }

    function getCurrentMode() {
        if (sc.getCurrentMode) {
            return sc.getCurrentMode()
        }
        if (sc.getMode) {
            return sc.getMode()
        }
        return 0
    }

    function getModeName(mode) {
        switch(mode) {
        case 0:
            return "自动模式"
        case 1:
            return "横向通行"
        case 2:
            return "纵向通行"
        case 3:
            return "全灭模式"
        case 4:
            return "夜间模式"
        default:
            return "未知模式"
        }
    }

    function updateTitle(mode) {
        var crossingId = getCrossingId()
        var modeValue = (mode !== undefined) ? mode : getCurrentMode()
        var modeName = getModeName(modeValue)
        title = qsTr("路口 ") + crossingId + " - " + modeName
        console.log("更新标题:", title)
    }

    Menu {
        id: contextMenu

        MenuItem {
            text: "自动模式"
            onTriggered: {
                console.log("切换到自动模式")
                sc.setMode(0)
            }
        }

        MenuItem {
            text: "横向通行模式"
            onTriggered: {
                console.log("切换到横向通行模式")
                sc.setMode(1)
            }
        }

        MenuItem {
            text: "纵向通行模式"
            onTriggered: {
                console.log("切换到纵向通行模式")
                sc.setMode(2)
            }
        }

        MenuSeparator {}

        MenuItem {
            text: "全灭模式"
            onTriggered: {
                console.log("切换到全灭模式")
                sc.setMode(3)
            }
        }

        MenuItem {
            text: "夜间模式"
            onTriggered: {
                console.log("切换到夜间模式")
                sc.setMode(4)
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        propagateComposedEvents: true

        onClicked: {
            if (mouse.button === Qt.RightButton) {
                contextMenu.popup()
            }
        }
    }
}