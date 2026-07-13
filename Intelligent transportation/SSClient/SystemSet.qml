import QtQuick
import QtQuick.Controls

Item {
    // IP地址输入框
    Rectangle {
        id: ip
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.leftMargin: 6
        anchors.topMargin: 20
        border.width: 1
        radius: 3
        border.color: "#C8C8C8"
        width: (parent.width - 24) * 0.6
        height: 30
        TextInput {
            id: ipTI
            anchors.fill: parent
            anchors.margins: 6
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            font.pixelSize: 16
        }
    }

    // 端口号输入框
    Rectangle {
        id: port
        anchors.left: ip.right
        anchors.top: parent.top
        anchors.leftMargin: 6
        anchors.topMargin: 20
        border.width: 1
        radius: 3
        border.color: "#C8C8C8"
        width: (parent.width - 24) * 0.2
        height: 30
        TextInput {
            id: portTI
            anchors.fill: parent
            anchors.margins: 6
            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            font.pixelSize: 16
            validator: IntValidator { bottom: 1; top: 65535 }
        }
    }

    // 保存并连接按钮
    Button {
        text: "保存并连接"
        anchors.left: port.right
        anchors.top: parent.top
        anchors.leftMargin: 6
        anchors.topMargin: 20
        width: (parent.width - 24) * 0.2
        font.pixelSize: 16
        height: 30
        onClicked: {
            nc.connectToServer(ipTI.text, Number(portTI.text))
        }
    }

    // 连接状态显示区域
    Rectangle {
        id: statusRect
        anchors.left: parent.left
        anchors.top: ip.bottom
        anchors.leftMargin: 6
        anchors.topMargin: 20
        width: parent.width - 12
        height: 50
        border.width: 1
        radius: 5
        border.color: statusBorderColor
        color: statusBgColor

        Row {
            anchors.centerIn: parent
            spacing: 10

            // 状态指示灯
            Rectangle {
                width: 12
                height: 12
                radius: 6
                color: isConnected ? "#4CAF50" : "#F44336"
                anchors.verticalCenter: parent.verticalCenter
            }

            // 状态文字
            Text {
                text: isConnected ? "已连接到服务器" : "未连接服务器"
                font.pixelSize: 14
                font.bold: true
                color: isConnected ? "#4CAF50" : "#F44336"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }

    // 连接状态属性（响应式绑定）
    property bool isConnected: nc ? nc.isConnected() : false

    // 状态背景颜色
    property color statusBgColor: isConnected ? "#E8F5E9" : "#FFEBEE"
    property color statusBorderColor: isConnected ? "#4CAF50" : "#F44336"

    // 组件加载完成时初始化
    Component.onCompleted: {
        // 获取当前配置
        ipTI.text = nc.getCurrentIp() || "127.0.0.1"
        portTI.text = nc.getCurrentPort() || "10086"

        // 连接状态变化信号
        if (nc) {
            nc.connectionStatusChanged.connect(onConnectionStatusChanged)
            // 立即更新一次状态
            onConnectionStatusChanged(nc.isConnected())
        }
    }

    // 连接状态变化处理函数
    function onConnectionStatusChanged(connected) {
        console.log("连接状态变化: " + (connected ? "已连接" : "未连接"))
        isConnected = connected
    }
}