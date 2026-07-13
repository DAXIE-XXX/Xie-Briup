import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Page {
    id: loginPage
    background: Rectangle { color: "#1a1a2e" }

    ColumnLayout {
        anchors.centerIn: parent
        width: Math.min(parent.width * 0.85, 400)
        spacing: 20

        // 标题
        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 8
            Label {
                text: "🏠 智能家居网关"
                font.pixelSize: 28
                font.bold: true
                color: "#e94560"
                Layout.alignment: Qt.AlignHCenter
            }
            Label {
                text: "室内空气质量监控系统"
                font.pixelSize: 13
                color: "#a0a0a0"
                Layout.alignment: Qt.AlignHCenter
            }
        }

        // 连接状态指示
        Rectangle {
            Layout.preferredWidth: 120
            Layout.preferredHeight: 32
            Layout.alignment: Qt.AlignHCenter
            radius: 16
            color: controller.connected ? "#1a472a" : "#472a1a"
            Label {
                anchors.centerIn: parent
                text: controller.connected ? "● 已连接" : "○ 未连接"
                color: controller.connected ? "#4caf50" : "#ff9800"
                font.pixelSize: 12
            }
        }

        // IP 输入
        GroupBox {
            title: "服务器地址"
            Layout.fillWidth: true
            background: Rectangle {
                color: "#16213e"
                radius: 10
                border.color: "#0f3460"
            }
            label: Label {
                text: "服务器地址"
                color: "#e0e0e0"
                font.bold: true
                font.pixelSize: 14
            }
            ColumnLayout {
                anchors.fill: parent
                spacing: 10
                RowLayout {
                    Label { text: "IP 地址"; color: "#e0e0e0"; Layout.preferredWidth: 70 }
                    TextField {
                        id: editIP
                        text: "192.168.1.100"
                        color: "#e0e0e0"
                        Layout.fillWidth: true
                        background: Rectangle {
                            color: "#1a1a2e"
                            radius: 8
                            border.color: editIP.activeFocus ? "#e94560" : "#0f3460"
                        }
                    }
                }
                RowLayout {
                    Label { text: "端    口"; color: "#e0e0e0"; Layout.preferredWidth: 70 }
                    TextField {
                        id: editPort
                        text: "10086"
                        color: "#e0e0e0"
                        Layout.fillWidth: true
                        inputMethodHints: Qt.ImhDigitsOnly
                        background: Rectangle {
                            color: "#1a1a2e"
                            radius: 8
                            border.color: editPort.activeFocus ? "#e94560" : "#0f3460"
                        }
                    }
                }
            }
        }

        // 按钮
        RowLayout {
            Layout.fillWidth: true
            spacing: 16
            Button {
                text: controller.connected ? "断开" : "连接服务器"
                Layout.fillWidth: true
                Layout.preferredHeight: 48
                font.bold: true
                font.pixelSize: 15
                onClicked: {
                    if (controller.connected) {
                        controller.disconnectFromServer()
                    } else {
                        controller.connectToServer(editIP.text, parseInt(editPort.text) || 10086)
                    }
                }
                background: Rectangle {
                    radius: 8
                    color: parent.hovered ? "#c73e54" : "#e94560"
                }
                contentItem: Label {
                    text: parent.text
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: parent.font
                }
            }
        }

        // 进入监控面板
        Button {
            text: "进入监控面板 →"
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            enabled: controller.connected
            opacity: enabled ? 1.0 : 0.4
            font.pixelSize: 14
            onClicked: {
                stackView.push("qrc:/qml/DashboardPage.qml")
                    appWindow.switchToTab(0)
            }
            background: Rectangle {
                radius: 8
                color: parent.hovered ? "#1a3a5e" : "#0f3460"
            }
            contentItem: Label {
                text: parent.text
                color: "white"
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                font: parent.font
            }
        }
    }
}
