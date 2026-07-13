import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: dashboardPage
    background: Rectangle { color: "#1a1a2e" }
    title: "实时监控"

    // 顶栏
    header: ToolBar {
        background: Rectangle { color: "#16213e" }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            anchors.rightMargin: 12
            ToolButton {
                text: "←"
                font.pixelSize: 20
                onClicked: appWindow.switchToTab(0)
                background: Rectangle { color: "transparent" }
                contentItem: Label { text: "←"; color: "#e0e0e0"; font.pixelSize: 22 }
            }
            Label {
                text: "实时监控面板"
                font.pixelSize: 18
                font.bold: true
                color: "#e0e0e0"
                Layout.fillWidth: true
            }
            // 连接状态指示
            Rectangle {
                width: 10; height: 10; radius: 5
                color: controller.connected ? "#4caf50" : "#ff5252"
            }
        }
    }

    // 传感器卡片网格
    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        GridLayout {
            anchors.fill: parent
            anchors.margins: 12
            columns: parent.width > 480 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12

            // CO2
            SensorCard {
                title: "CO₂"
                value: controller.co2.toFixed(0) + " ppm"
                unit: ""
                icon: "💨"
                accentColor: controller.co2 > controller.co2Max ? "#e94560" : "#4caf50"
                onRequested: controller.requestSensor(20001)
            }

            // 温度
            SensorCard {
                title: "温度"
                value: controller.temperature.toFixed(1)
                unit: "°C"
                icon: "🌡️"
                accentColor: (controller.temperature > controller.tempMax ||
                        controller.temperature < controller.tempMin) ? "#e94560" : "#4caf50"
                onRequested: controller.requestSensor(20002)
            }

            // 湿度
            SensorCard {
                title: "湿度"
                value: controller.humidity.toFixed(1)
                unit: "%"
                icon: "💧"
                accentColor: "#42a5f5"
                onRequested: controller.requestSensor(20008)
            }

            // PM2.5
            SensorCard {
                title: "PM2.5"
                value: controller.pm25.toFixed(0)
                unit: "µg/m³"
                icon: "🌫️"
                accentColor: controller.pm25 > controller.pm25Max ? "#e94560" : "#4caf50"
                onRequested: controller.requestSensor(20004)
            }

            // 甲烷
            SensorCard {
                title: "甲烷"
                value: controller.methane ? "⚠️ 异常" : "正常"
                unit: ""
                icon: "⛽"
                accentColor: controller.methane ? "#e94560" : "#4caf50"
                onRequested: controller.requestSensor(20005)
            }

            // 光照
            SensorCard {
                title: "光照"
                value: controller.light.toFixed(0)
                unit: "lux"
                icon: "💡"
                accentColor: "#ffa726"
                onRequested: controller.requestSensor(20006)
            }

            // 烟雾
            SensorCard {
                title: "烟雾"
                value: controller.smoke ? "⚠️ 异常" : "正常"
                unit: ""
                icon: "💨"
                accentColor: controller.smoke ? "#e94560" : "#4caf50"
                onRequested: controller.requestSensor(20007)
            }

            // 火焰
            SensorCard {
                title: "火焰"
                value: controller.fire ? "🔥 检测到火焰!" : "正常"
                unit: ""
                icon: "🔥"
                accentColor: controller.fire ? "#e94560" : "#4caf50"
                onRequested: controller.requestSensor(20003)
            }
        }
    }

    // 传感器卡片组件
    component SensorCard: Rectangle {
        property string title: ""
        property string value: ""
        property string unit: ""
        property string icon: ""
        property color accentColor: "#4caf50"
        signal requested()

        Layout.fillWidth: true
        Layout.preferredHeight: 120
        radius: 12
        color: "#16213e"
        border.color: "#0f3460"

        MouseArea {
            anchors.fill: parent
            onClicked: parent.requested()
        }

        RowLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            // 图标区
            Rectangle {
                width: 56; height: 56; radius: 28
                color: "#1a1a2e"
                Label {
                    anchors.centerIn: parent
                    text: icon
                    font.pixelSize: 28
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 4
                Label {
                    text: title
                    color: "#a0a0a0"
                    font.pixelSize: 13
                }
                RowLayout {
                    spacing: 4
                    Label {
                        text: value
                        color: parent.parent.parent.accentColor
                        font.pixelSize: 24
                        font.bold: true
                    }
                    Label {
                        text: unit
                        color: "#a0a0a0"
                        font.pixelSize: 14
                    }
                }
            }

            // 刷新按钮
            Label {
                text: "↻"
                font.pixelSize: 20
                color: "#a0a0a0"
            }
        }
    }
}
