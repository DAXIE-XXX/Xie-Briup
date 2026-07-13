import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("智慧交通管理系统")

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ========== 左侧导航栏（简洁图标版） ==========
        Rectangle {
            id: leftNavBar
            Layout.preferredWidth: parent.width * 0.22
            Layout.fillHeight: true
            color: "#2C3E50"  // 深色背景
            z: 1

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 20

                // ========== 水印图片区域 ==========
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "transparent"

                    Image {
                        anchors.centerIn: parent
                        width: parent.width - 20
                        height: width * 0.6
                        fillMode: Image.PreserveAspectFit
                        source: "images/briup.png"
                        opacity: 0.9
                    }
                }

                // ========== 分隔线 ==========
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 1
                    color: "#4A6A8A"
                }

                // ========== 综合信息按钮 ==========
                Button {
                    id: btnMapAll
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    contentItem: Column {
                        spacing: 5
                        Image {
                            source: "images/home.png"
                            width: 32
                            height: 32
                            fillMode: Image.PreserveAspectFit
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "综合信息"
                            font.pixelSize: 12
                            color: "white"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    background: Rectangle {
                        color: btnMapAll.pressed ? "#1A6DB0" : "transparent"
                        radius: 8
                    }
                    onClicked: sv.setCurrentIndex(0)
                }

                // ========== 路口设置按钮 ==========
                Button {
                    id: btnCrossingSet
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    contentItem: Column {
                        spacing: 5
                        Image {
                            source: "images/control.png"
                            width: 32
                            height: 32
                            fillMode: Image.PreserveAspectFit
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "路口设置"
                            font.pixelSize: 12
                            color: "white"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    background: Rectangle {
                        color: btnCrossingSet.pressed ? "#1A6DB0" : "transparent"
                        radius: 8
                    }
                    onClicked: sv.setCurrentIndex(1)
                }

                // ========== 车辆设置按钮 ==========
                Button {
                    id: btnCarSet
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    contentItem: Column {
                        spacing: 5
                        Image {
                            source: "images/setting.png"
                            width: 32
                            height: 32
                            fillMode: Image.PreserveAspectFit
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "车辆设置"
                            font.pixelSize: 12
                            color: "white"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    background: Rectangle {
                        color: btnCarSet.pressed ? "#1A6DB0" : "transparent"
                        radius: 8
                    }
                    onClicked: sv.setCurrentIndex(2)
                }

                // ========== 系统设置按钮 ==========
                Button {
                    id: btnSystemSet
                    Layout.fillWidth: true
                    Layout.preferredHeight: 70
                    contentItem: Column {
                        spacing: 5
                        Image {
                            source: "images/connect.png"
                            width: 32
                            height: 32
                            fillMode: Image.PreserveAspectFit
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                        Text {
                            text: "系统设置"
                            font.pixelSize: 12
                            color: "white"
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                    background: Rectangle {
                        color: btnSystemSet.pressed ? "#1A6DB0" : "transparent"
                        radius: 8
                    }
                    onClicked: sv.setCurrentIndex(3)
                }

                Item {
                    Layout.fillHeight: true
                }
            }
        }

        // ========== 右侧内容区域 ==========
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            spacing: 0

            SwipeView {
                id: sv
                Layout.fillHeight: true
                Layout.fillWidth: true
                currentIndex: 0

                Map { id: pageMap }
                CrossingSet { id: pageCrossingSet }
                CarSet { id: pageCarSet }
                SystemSet { id: pageSystemSet }
            }

            PageIndicator {
                id: indicator
                Layout.alignment: Qt.AlignHCenter
                Layout.bottomMargin: 5
                count: sv.count
                currentIndex: sv.currentIndex
            }
        }
    }
}