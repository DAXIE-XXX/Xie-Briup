import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

GroupBox {
    id: root
    title: "路口控制"

    // 路口0状态
    property string modeName0: "等待连接"
    // 路口1状态
    property string modeName1: "等待连接"

    GridLayout {
        anchors.fill: parent
        columns: 2
        columnSpacing: 15
        rowSpacing: 10

        // ========== 路口0控制区域 ==========
        GroupBox {
            title: "路口 0"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width / 2

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 8
                rowSpacing: 5

                Text {
                    id: modeText0
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 2
                    Layout.columnSpan: 2
                    text: root.modeName0
                    font.pixelSize: parent.height * 0.08
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    font.bold: true
                }

                Button {
                    text: "自动模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName0 = getModeText(0)
                        nc.sendCrossingMode(0, 0)
                    }
                }

                Button {
                    text: "水平模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName0 = getModeText(1)
                        nc.sendCrossingMode(0, 1)
                    }
                }

                Button {
                    text: "垂直模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName0 = getModeText(2)
                        nc.sendCrossingMode(0, 2)
                    }
                }

                Button {
                    text: "夜间模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName0 = getModeText(4)
                        nc.sendCrossingMode(0, 4)
                    }
                }

                Button {
                    text: "禁行模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName0 = getModeText(3)
                        nc.sendCrossingMode(0, 3)
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                }
            }
        }

        // ========== 路口1控制区域 ==========
        GroupBox {
            title: "路口 1"
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width / 2

            GridLayout {
                anchors.fill: parent
                columns: 2
                columnSpacing: 8
                rowSpacing: 5

                Text {
                    id: modeText1
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.preferredHeight: 2
                    Layout.columnSpan: 2
                    text: root.modeName1
                    font.pixelSize: parent.height * 0.08
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    font.bold: true
                }

                Button {
                    text: "自动模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName1 = getModeText(0)
                        nc.sendCrossingMode(1, 0)
                    }
                }

                Button {
                    text: "水平模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName1 = getModeText(1)
                        nc.sendCrossingMode(1, 1)
                    }
                }

                Button {
                    text: "垂直模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName1 = getModeText(2)
                        nc.sendCrossingMode(1, 2)
                    }
                }

                Button {
                    text: "夜间模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName1 = getModeText(4)
                        nc.sendCrossingMode(1, 4)
                    }
                }

                Button {
                    text: "禁行模式"
                    Layout.preferredHeight: 2
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    onClicked: {
                        root.modeName1 = getModeText(3)
                        nc.sendCrossingMode(1, 3)
                    }
                }

                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                }
            }
        }
    }

    // 模式名称转换函数
    function getModeText(mode) {
        var names = ["自动模式", "水平模式", "垂直模式", "禁行模式", "夜间模式"]
        return names[mode] || "未知模式"
    }

    // 接收服务器返回的状态更新
    Component.onCompleted: {
        nc.modeStatusReceived.connect(function(id, mode) {
            console.log("收到服务器响应: 路口", id, "模式", mode)
            if (id === 0) {
                root.modeName0 = getModeText(mode)
            } else if (id === 1) {
                root.modeName1 = getModeText(mode)
            }
        })
    }
}