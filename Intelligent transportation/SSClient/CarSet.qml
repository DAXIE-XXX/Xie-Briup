import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    // 定义车辆0的属性变量
    property string car0LightText: "关闭"
    property string car0AlarmText: "关闭"
    property string car0StartText: "停止"
    property string car0LockText: "解锁"
    property string car0EnduranceText: "0.0KM"
    property string car0MileageText: "0.0KM"
    property string car0WarningText: "--"

    // 车辆0双闪闪烁控制
    property bool car0AlarmBlink: false

    // 定义车辆1的属性变量
    property string car1LightText: "关闭"
    property string car1AlarmText: "关闭"
    property string car1StartText: "停止"
    property string car1LockText: "解锁"
    property string car1EnduranceText: "0.0KM"
    property string car1MileageText: "0.0KM"
    property string car1WarningText: "--"

    // 车辆1双闪闪烁控制
    property bool car1AlarmBlink: false

    // 定义车辆2的属性变量
    property string car2LightText: "关闭"
    property string car2AlarmText: "关闭"
    property string car2StartText: "停止"
    property string car2LockText: "解锁"
    property string car2EnduranceText: "0.0KM"
    property string car2MileageText: "0.0KM"
    property string car2WarningText: "--"

    // 车辆2双闪闪烁控制
    property bool car2AlarmBlink: false

    // 车辆0双闪闪烁定时器
    Timer {
        id: alarmTimer0
        interval: 500
        repeat: true
        running: car0AlarmText === "开启"
        onTriggered: car0AlarmBlink = !car0AlarmBlink
    }

    // 车辆1双闪闪烁定时器
    Timer {
        id: alarmTimer1
        interval: 500
        repeat: true
        running: car1AlarmText === "开启"
        onTriggered: car1AlarmBlink = !car1AlarmBlink
    }

    // 车辆2双闪闪烁定时器
    Timer {
        id: alarmTimer2
        interval: 500
        repeat: true
        running: car2AlarmText === "开启"
        onTriggered: car2AlarmBlink = !car2AlarmBlink
    }

    // 定时器强制刷新（最可靠）
    Timer {
        interval: 100
        running: true
        repeat: true
        onTriggered: {
            dataUpdateSlot()
        }
    }

    // 三辆车横向排列
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        // ==================== 车辆 0 ====================
        GroupBox {
            id: car0
            title: "车0"
            Layout.fillHeight: true
            Layout.fillWidth: true
            font.pixelSize: 16
            font.bold: true

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 8
                rowSpacing: 6

                // 开灯
                Button {
                    text: "开灯"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 开灯")
                        car0LightText = "开启"
                        nc.sendCarLightSet(0, true)
                    }
                }
                // 关灯
                Button {
                    text: "关灯"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 关灯")
                        car0LightText = "关闭"
                        nc.sendCarLightSet(0, false)
                    }
                }

                // 开双闪
                Button {
                    text: "开双闪"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 开双闪")
                        car0AlarmText = "开启"
                        nc.sendCarAlarmSet(0, true)
                    }
                }
                // 关双闪
                Button {
                    text: "关双闪"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 关双闪")
                        car0AlarmText = "关闭"
                        nc.sendCarAlarmSet(0, false)
                    }
                }

                // 启动
                Button {
                    text: "启动"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 启动")
                        car0StartText = "启动"
                        // 使用本地 API，立即生效
                        dc.startCarLocally(0)
                    }
                }
                // 停止
                Button {
                    text: "停止"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 停止")
                        car0StartText = "停止"
                        dc.stopCarLocally(0)
                    }
                }

                // 锁定
                Button {
                    text: "锁定"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 锁定")
                        car0LockText = "锁定"
                        nc.sendCarLockSet(0, true)
                    }
                }
                // 解锁
                Button {
                    text: "解锁"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车0 解锁")
                        car0LockText = "解锁"
                        nc.sendCarLockSet(0, false)
                    }
                }

                // 大灯状态标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "大灯状态"
                        font.pixelSize: 12
                    }
                }
                // 大灯状态值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car0LightText === "开启" ? "#FFF9C4" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car0LightText
                        font.pixelSize: 12
                        font.bold: true
                        color: car0LightText === "开启" ? "#FF9800" : "#666666"
                    }
                }

                // 双闪状态标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "双闪状态"
                        font.pixelSize: 12
                    }
                }
                // 双闪状态值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car0AlarmText === "开启" ? (car0AlarmBlink ? "#FFCC80" : "#FFE0B2") : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car0AlarmText
                        font.pixelSize: 12
                        font.bold: true
                        color: car0AlarmText === "开启" ? "#F44336" : "#666666"
                    }
                }

                // 启动状态标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "启动状态"
                        font.pixelSize: 12
                    }
                }
                // 启动状态值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car0StartText === "启动" ? "#C8E6C9" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car0StartText
                        font.pixelSize: 12
                        font.bold: true
                        color: car0StartText === "启动" ? "#4CAF50" : "#F44336"
                    }
                }

                // 锁定状态标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "锁定状态"
                        font.pixelSize: 12
                    }
                }
                // 锁定状态值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car0LockText === "锁定" ? "#FFCDD2" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car0LockText
                        font.pixelSize: 12
                        font.bold: true
                        color: car0LockText === "锁定" ? "#F44336" : "#4CAF50"
                    }
                }

                // 续航标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "续航"
                        font.pixelSize: 12
                    }
                }
                // 续航值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#E8EAF6"
                    Text {
                        anchors.centerIn: parent
                        text: car0EnduranceText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#2196F3"
                    }
                }

                // 里程标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "里程"
                        font.pixelSize: 12
                    }
                }
                // 里程值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#E8EAF6"
                    Text {
                        anchors.centerIn: parent
                        text: car0MileageText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#4CAF50"
                    }
                }

                // 警告标签
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: "警告"
                        font.pixelSize: 12
                    }
                }
                // 警告值
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car0WarningText !== "--" ? "#FFCDD2" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car0WarningText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#f44336"
                    }
                }
            }
        }

        // ==================== 车辆 1 ====================
        GroupBox {
            id: car1
            title: "车1"
            Layout.fillHeight: true
            Layout.fillWidth: true
            font.pixelSize: 16
            font.bold: true

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 8
                rowSpacing: 6

                Button {
                    text: "开灯"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 开灯")
                        car1LightText = "开启"
                        nc.sendCarLightSet(1, true)
                    }
                }
                Button {
                    text: "关灯"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 关灯")
                        car1LightText = "关闭"
                        nc.sendCarLightSet(1, false)
                    }
                }

                Button {
                    text: "开双闪"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 开双闪")
                        car1AlarmText = "开启"
                        nc.sendCarAlarmSet(1, true)
                    }
                }
                Button {
                    text: "关双闪"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 关双闪")
                        car1AlarmText = "关闭"
                        nc.sendCarAlarmSet(1, false)
                    }
                }

                Button {
                    text: "启动"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 启动")
                        car1StartText = "启动"
                        dc.startCarLocally(1)
                    }
                }
                Button {
                    text: "停止"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 停止")
                        car1StartText = "停止"
                        dc.stopCarLocally(1)
                    }
                }

                Button {
                    text: "锁定"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 锁定")
                        car1LockText = "锁定"
                        nc.sendCarLockSet(1, true)
                    }
                }
                Button {
                    text: "解锁"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车1 解锁")
                        car1LockText = "解锁"
                        nc.sendCarLockSet(1, false)
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "大灯状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car1LightText === "开启" ? "#FFF9C4" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car1LightText
                        font.pixelSize: 12
                        font.bold: true
                        color: car1LightText === "开启" ? "#FF9800" : "#666666"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "双闪状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car1AlarmText === "开启" ? (car1AlarmBlink ? "#FFCC80" : "#FFE0B2") : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car1AlarmText
                        font.pixelSize: 12
                        font.bold: true
                        color: car1AlarmText === "开启" ? "#F44336" : "#666666"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "启动状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car1StartText === "启动" ? "#C8E6C9" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car1StartText
                        font.pixelSize: 12
                        font.bold: true
                        color: car1StartText === "启动" ? "#4CAF50" : "#F44336"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "锁定状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car1LockText === "锁定" ? "#FFCDD2" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car1LockText
                        font.pixelSize: 12
                        font.bold: true
                        color: car1LockText === "锁定" ? "#F44336" : "#4CAF50"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "续航"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#E8EAF6"
                    Text {
                        anchors.centerIn: parent
                        text: car1EnduranceText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#2196F3"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "里程"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#E8EAF6"
                    Text {
                        anchors.centerIn: parent
                        text: car1MileageText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#4CAF50"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "警告"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car1WarningText !== "--" ? "#FFCDD2" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car1WarningText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#f44336"
                    }
                }
            }
        }

        // ==================== 车辆 2 ====================
        GroupBox {
            id: car2
            title: "车2"
            Layout.fillHeight: true
            Layout.fillWidth: true
            font.pixelSize: 16
            font.bold: true

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 8
                rowSpacing: 6

                Button {
                    text: "开灯"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 开灯")
                        car2LightText = "开启"
                        nc.sendCarLightSet(2, true)
                    }
                }
                Button {
                    text: "关灯"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 关灯")
                        car2LightText = "关闭"
                        nc.sendCarLightSet(2, false)
                    }
                }

                Button {
                    text: "开双闪"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 开双闪")
                        car2AlarmText = "开启"
                        nc.sendCarAlarmSet(2, true)
                    }
                }
                Button {
                    text: "关双闪"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 关双闪")
                        car2AlarmText = "关闭"
                        nc.sendCarAlarmSet(2, false)
                    }
                }

                Button {
                    text: "启动"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 启动")
                        car2StartText = "启动"
                        dc.startCarLocally(2)
                    }
                }
                Button {
                    text: "停止"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 停止")
                        car2StartText = "停止"
                        dc.stopCarLocally(2)
                    }
                }

                Button {
                    text: "锁定"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 锁定")
                        car2LockText = "锁定"
                        nc.sendCarLockSet(2, true)
                    }
                }
                Button {
                    text: "解锁"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 35
                    onClicked: {
                        console.log("车2 解锁")
                        car2LockText = "解锁"
                        nc.sendCarLockSet(2, false)
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "大灯状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car2LightText === "开启" ? "#FFF9C4" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car2LightText
                        font.pixelSize: 12
                        font.bold: true
                        color: car2LightText === "开启" ? "#FF9800" : "#666666"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "双闪状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car2AlarmText === "开启" ? (car2AlarmBlink ? "#FFCC80" : "#FFE0B2") : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car2AlarmText
                        font.pixelSize: 12
                        font.bold: true
                        color: car2AlarmText === "开启" ? "#F44336" : "#666666"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "启动状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car2StartText === "启动" ? "#C8E6C9" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car2StartText
                        font.pixelSize: 12
                        font.bold: true
                        color: car2StartText === "启动" ? "#4CAF50" : "#F44336"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "锁定状态"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car2LockText === "锁定" ? "#FFCDD2" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car2LockText
                        font.pixelSize: 12
                        font.bold: true
                        color: car2LockText === "锁定" ? "#F44336" : "#4CAF50"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "续航"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#E8EAF6"
                    Text {
                        anchors.centerIn: parent
                        text: car2EnduranceText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#2196F3"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "里程"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#E8EAF6"
                    Text {
                        anchors.centerIn: parent
                        text: car2MileageText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#4CAF50"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: "#f5f5f5"
                    Text { anchors.centerIn: parent; text: "警告"; font.pixelSize: 12 }
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    border.width: 1
                    border.color: "#ccc"
                    radius: 3
                    color: car2WarningText !== "--" ? "#FFCDD2" : "#f5f5f5"
                    Text {
                        anchors.centerIn: parent
                        text: car2WarningText
                        font.pixelSize: 12
                        font.bold: true
                        color: "#f44336"
                    }
                }
            }
        }
    }

    // 数据更新槽函数
    function dataUpdateSlot()  {
        if (!dc) {
            console.log("dataUpdateSlot: dc is null")
            return
        }

        console.log("Updating car data - endurance0:" + dc.getCarEndurance(0))

        // 车辆0
        car0EnduranceText = dc.getCarEndurance(0).toFixed(1) + "KM"
        car0MileageText = dc.getCarMileage(0).toFixed(1) + "KM"

        // 车辆1
        car1EnduranceText = dc.getCarEndurance(1).toFixed(1) + "KM"
        car1MileageText = dc.getCarMileage(1).toFixed(1) + "KM"

        // 车辆2
        car2EnduranceText = dc.getCarEndurance(2).toFixed(1) + "KM"
        car2MileageText = dc.getCarMileage(2).toFixed(1) + "KM"
    }

    // 组件加载完成时初始化
    Component.onCompleted: {
        console.log("CarSet 组件加载完成")
        dataUpdateSlot()
    }
}