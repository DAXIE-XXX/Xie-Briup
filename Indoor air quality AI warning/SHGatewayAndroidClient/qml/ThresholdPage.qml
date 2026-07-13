import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: thresholdPage
    background: Rectangle { color: "#1a1a2e" }
    title: "阈值配置"

    header: ToolBar {
        background: Rectangle { color: "#16213e" }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 12
            ToolButton {
                text: "←"
                onClicked: appWindow.switchToTab(3)
                contentItem: Label { text: "←"; color: "#e0e0e0"; font.pixelSize: 22 }
            }
            Label {
                text: "告警阈值配置"; font.pixelSize: 18; font.bold: true; color: "#e0e0e0"
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 12

            // ===== CO2 =====
            SettingCard {
                title: "CO₂ 浓度上限"
                subtitle: "超过此值触发风扇+报警灯联动"
                unit: "ppm"
                minValue: 400; maxValue: 5000
                initValue: controller.co2Max
                onChanged: function(v) { co2MaxVal = v }
            }

            // ===== PM2.5 =====
            SettingCard {
                title: "PM2.5 浓度上限"
                subtitle: "超过此值触发风扇+报警灯联动"
                unit: "µg/m³"
                minValue: 10; maxValue: 500
                initValue: controller.pm25Max
                onChanged: function(v) { pm25MaxVal = v }
            }

            // ===== 温度上限 =====
            SettingCard {
                title: "温度上限"
                subtitle: "超过此温度触发告警"
                unit: "°C"
                minValue: 20; maxValue: 50
                initValue: controller.tempMax
                onChanged: function(v) { tempMaxVal = v }
            }

            // ===== 温度下限 =====
            SettingCard {
                title: "温度下限"
                subtitle: "低于此温度触发告警"
                unit: "°C"
                minValue: 0; maxValue: 25
                initValue: controller.tempMin
                onChanged: function(v) { tempMinVal = v }
            }

            // ===== 甲烷检测 =====
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 70
                radius: 12; color: "#16213e"; border.color: "#0f3460"
                RowLayout {
                    anchors.fill: parent; anchors.margins: 16
                    ColumnLayout {
                        Layout.fillWidth: true
                        Label {
                            text: "⛽ 甲烷检测"
                            color: "#e0e0e0"; font.pixelSize: 16; font.bold: true
                        }
                        Label {
                            text: "是否启用甲烷泄漏检测"
                            color: "#a0a0a0"; font.pixelSize: 12
                        }
                    }
                    Switch {
                        id: methaneSwitch
                        checked: controller.methaneEnable
                    }
                }
            }

            // ===== 告警冷却 =====
            SettingCard {
                title: "告警冷却时间"
                subtitle: "同一传感器两次触发的最小间隔"
                unit: "秒"
                minValue: 5; maxValue: 300
                initValue: controller.cooldownSeconds
                onChanged: function(v) { cooldownVal = v }
            }

            // ===== 风扇延迟 =====
            SettingCard {
                title: "风扇关闭延迟"
                subtitle: "数值正常后风扇继续运行时间"
                unit: "秒"
                minValue: 0; maxValue: 300
                initValue: controller.fanCooldownSeconds
                onChanged: function(v) { fanCooldownVal = v }
            }

            // ===== 按钮 =====
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Button {
                    text: "恢复默认"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    onClicked: {
                        co2MaxVal = 1000; pm25MaxVal = 50
                        tempMaxVal = 30; tempMinVal = 18
                        methaneSwitch.checked = true
                        cooldownVal = 30; fanCooldownVal = 30
                    }
                    background: Rectangle { radius: 8; color: "#2d2d3a" }
                    contentItem: Label { text: "恢复默认"; color: "#e0e0e0"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
                Button {
                    text: "应用阈值"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 44
                    onClicked: {
                        controller.applyThreshold(
                            co2MaxVal, pm25MaxVal,
                            tempMaxVal, tempMinVal,
                            methaneSwitch.checked,
                            cooldownVal, fanCooldownVal
                        )
                    }
                    background: Rectangle { radius: 8; color: "#e94560" }
                    contentItem: Label { text: "应用阈值"; color: "white"; font.bold: true; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                }
            }

            // 状态提示
            Label {
                id: thresholdStatus
                text: ""
                color: "#4caf50"
                font.pixelSize: 13
                Layout.alignment: Qt.AlignHCenter
            }

            Connections {
                target: controller
                function onThresholdApplied(success) {
                    thresholdStatus.text = success ? "✅ 阈值配置成功" : "❌ 配置失败"
                    thresholdStatus.color = success ? "#4caf50" : "#e94560"
                }
            }
        }
    }

    // 内部变量
    property int co2MaxVal: controller.co2Max
    property int pm25MaxVal: controller.pm25Max
    property double tempMaxVal: controller.tempMax
    property double tempMinVal: controller.tempMin
    property int cooldownVal: controller.cooldownSeconds
    property int fanCooldownVal: controller.fanCooldownSeconds

    // 滑块设置卡片组件
    component SettingCard: Rectangle {
        property string title: ""; property string subtitle: ""; property string unit: ""
        property int minValue: 0; property int maxValue: 100
        property int initValue: 0
        property var onChanged: function(v) {}

        Layout.fillWidth: true
        Layout.preferredHeight: 110
        radius: 12; color: "#16213e"; border.color: "#0f3460"

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 8
            RowLayout {
                Layout.fillWidth: true
                ColumnLayout {
                    Layout.fillWidth: true
                    Label { text: title; color: "#e0e0e0"; font.pixelSize: 16; font.bold: true }
                    Label { text: subtitle; color: "#a0a0a0"; font.pixelSize: 12 }
                }
                Label {
                    text: sl.value.toFixed(0) + " " + unit
                    color: "#e94560"; font.pixelSize: 20; font.bold: true
                }
            }
            Slider {
                id: sl
                from: minValue; to: maxValue; value: initValue
                Layout.fillWidth: true
                onMoved: parent.parent.onChanged(sl.value)
            }
        }
    }
}
