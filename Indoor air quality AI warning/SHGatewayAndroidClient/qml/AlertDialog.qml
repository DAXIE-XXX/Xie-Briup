import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Dialog {
    id: alertDialog
    title: "🚨 告警通知"
    modal: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: parent
    width: Math.min(parent.width * 0.9, 380)
    padding: 20

    property string actionText: ""
    property double valueText: 0
    property string timestampText: ""

    background: Rectangle {
        color: "#1a1a2e"
        radius: 16
        border.color: "#e94560"
        border.width: 2
    }

    header: Label {
        text: "🚨 告警通知"
        font.pixelSize: 20
        font.bold: true
        color: "#e94560"
        padding: 12
        background: Rectangle { color: "#16213e"; radius: 12 }
    }

    contentItem: ColumnLayout {
        spacing: 16

        // 告警内容
        Label {
            text: actionText
            font.pixelSize: 16
            color: "#e0e0e0"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        // 数值
        Rectangle {
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 120
            Layout.preferredHeight: 60
            radius: 12
            color: "#2a1a1a"
            border.color: "#e94560"
            Label {
                anchors.centerIn: parent
                text: valueText.toFixed(1)
                font.pixelSize: 28
                font.bold: true
                color: "#e94560"
            }
        }

        // 时间
        Label {
            text: timestampText
            font.pixelSize: 11
            color: "#a0a0a0"
            Layout.alignment: Qt.AlignHCenter
        }

        // 确认按钮
        Button {
            text: "确认收到"
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            onClicked: {
                // 发送确认回执 (20051)
                controller.sendControl(20051, {
                    "action": actionText,
                    "success": true,
                    "message": "客户端已收到告警通知",
                    "timestamp": timestampText
                })
                alertDialog.close()
            }
            background: Rectangle {
                radius: 8
                color: "#e94560"
            }
            contentItem: Label {
                text: "确认收到"
                color: "white"
                font.bold: true
                font.pixelSize: 15
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }
}
