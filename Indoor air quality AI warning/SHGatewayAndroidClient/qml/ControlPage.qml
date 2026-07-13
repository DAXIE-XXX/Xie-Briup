import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: controlPage
    background: Rectangle { color: "#1a1a2e" }
    title: "设备控制"

    header: ToolBar {
        background: Rectangle { color: "#16213e" }
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 12
            ToolButton {
                text: "←"
                onClicked: appWindow.switchToTab(1)
                contentItem: Label { text: "←"; color: "#e0e0e0"; font.pixelSize: 22 }
            }
            Label {
                text: "设备控制"
                font.pixelSize: 18; font.bold: true; color: "#e0e0e0"
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        GridLayout {
            anchors.fill: parent
            anchors.margins: 12
            columns: parent.width > 480 ? 2 : 1
            columnSpacing: 12
            rowSpacing: 12

            // 风扇
            ControlSwitch {
                title: "🌀 风扇"; subtitle: "通风换气"
                initialState: false
                onType: 20013
            }
            // 报警器
            ControlSwitch {
                title: "🔔 报警器"; subtitle: "声音报警"
                initialState: false
                onType: 20014
            }
            // 报警灯
            ControlSwitch {
                title: "💡 报警灯"; subtitle: "视觉警示"
                initialState: false
                onType: 20015
            }
            // LED 亮度
            ControlSlider {
                title: "🔆 LED 亮度"
                subtitle: "照明控制"
                minValue: 0; maxValue: 255
                onSend: function(val) {
                    controller.sendControl(20011, { "id": 1, "light": val })
                }
            }
            // RGB 颜色
            GroupBox {
                title: "🎨 RGB 灯光"
                Layout.fillWidth: true
                background: Rectangle { color: "#16213e"; radius: 10; border.color: "#0f3460" }
                label: Label { text: "🎨 RGB 灯光"; color: "#e0e0e0"; font.bold: true }
                ColumnLayout {
                    anchors.fill: parent
                    spacing: 8
                    RowLayout {
                        Label { text: "R"; color: "#ff5252"; Layout.preferredWidth: 24 }
                        Slider {
                            id: sliderR; from: 0; to: 255; value: 0
                            Layout.fillWidth: true
                        }
                        Label { text: sliderR.value.toFixed(0); color: "#e0e0e0"; Layout.preferredWidth: 36 }
                    }
                    RowLayout {
                        Label { text: "G"; color: "#4caf50"; Layout.preferredWidth: 24 }
                        Slider {
                            id: sliderG; from: 0; to: 255; value: 0
                            Layout.fillWidth: true
                        }
                        Label { text: sliderG.value.toFixed(0); color: "#e0e0e0"; Layout.preferredWidth: 36 }
                    }
                    RowLayout {
                        Label { text: "B"; color: "#42a5f5"; Layout.preferredWidth: 24 }
                        Slider {
                            id: sliderB; from: 0; to: 255; value: 0
                            Layout.fillWidth: true
                        }
                        Label { text: sliderB.value.toFixed(0); color: "#e0e0e0"; Layout.preferredWidth: 36 }
                    }
                    Button {
                        text: "应用 RGB"
                        Layout.fillWidth: true
                        Layout.preferredHeight: 40
                        onClicked: {
                            controller.sendControl(20012, {
                                "red": sliderR.value,
                                "green": sliderG.value,
                                "blue": sliderB.value
                            })
                        }
                        background: Rectangle { radius: 8; color: "#0f3460" }
                        contentItem: Label { text: "应用 RGB"; color: "white"; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                    }
                }
            }
        }
    }

    // 开关组件
    component ControlSwitch: Rectangle {
        property string title: ""; property string subtitle: ""
        property bool initialState: false; property int onType: 0

        Layout.fillWidth: true
        Layout.preferredHeight: 90
        radius: 12; color: "#16213e"; border.color: "#0f3460"

        RowLayout {
            anchors.fill: parent; anchors.margins: 16
            ColumnLayout {
                Layout.fillWidth: true
                Label { text: title; color: "#e0e0e0"; font.pixelSize: 16; font.bold: true }
                Label { text: subtitle; color: "#a0a0a0"; font.pixelSize: 12 }
            }
            Switch {
                id: sw
                checked: initialState
                onCheckedChanged: {
                    controller.sendControl(onType, { "sw": sw.checked })
                }
            }
        }
    }

    // 滑块组件
    component ControlSlider: Rectangle {
        property string title: ""; property string subtitle: ""
        property int minValue: 0; property int maxValue: 255
        property var onSend: function(val) {}

        Layout.fillWidth: true
        Layout.preferredHeight: 110
        radius: 12; color: "#16213e"; border.color: "#0f3460"

        ColumnLayout {
            anchors.fill: parent; anchors.margins: 16; spacing: 8
            Label { text: title; color: "#e0e0e0"; font.pixelSize: 16; font.bold: true }
            Label { text: subtitle; color: "#a0a0a0"; font.pixelSize: 12 }
            RowLayout {
                Slider {
                    id: sl
                    from: minValue; to: maxValue; value: 128
                    Layout.fillWidth: true
                    onMoved: onSend(sl.value)
                }
                Label {
                    text: sl.value.toFixed(0)
                    color: "#e0e0e0"; Layout.preferredWidth: 36
                    font.pixelSize: 16; font.bold: true
                }
            }
        }
    }
}
