import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: appWindow
    visible: true
    width: 480
    height: 900
    title: "🏠 智能家居网关"

    readonly property color primaryColor: "#e94560"
    readonly property color bgDark: "#1a1a2e"
    readonly property color bgCard: "#16213e"
    readonly property color textPrimary: "#e0e0e0"
    readonly property color textSecondary: "#a0a0a0"
    readonly property color accentBlue: "#0f3460"

    minimumWidth: 320
    minimumHeight: 568

    Connections {
        target: controller
        function onAlertReceived(action, value, timestamp) {
            alertDialog.actionText = action
            alertDialog.valueText = value
            alertDialog.timestampText = timestamp
            alertDialog.open()
        }
    }

    // 供子页面访问的导航方法
    function switchToTab(index) {
        var urls = [
            "qrc:/qml/DashboardPage.qml",
            "qrc:/qml/ControlPage.qml",
            "qrc:/qml/ChartPage.qml",
            "qrc:/qml/ThresholdPage.qml"
        ]
        if (index >= 0 && index < urls.length && stackView.depth > 0) {
            stackView.replace(urls[index])
        }
    }
    function goBack() {
        if (stackView.depth > 1) {
            stackView.pop()
            // 根据栈顶URL估算当前tab
            tabBar.currentIndex = -1
        }
    }

    // 页面区域（占满上方）
    StackView {
        id: stackView
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: tabBar.top
        initialItem: "qrc:/qml/LoginPage.qml"
    }

    // 全局底部导航栏
    Rectangle {
        id: tabBar
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 56
        visible: stackView.depth > 1
        color: "#16213e"

        Row {
            anchors.fill: parent
            Repeater {
                model: ["📊 监控", "🎮 控制", "📈 图表", "⚙️ 阈值"]
                Item {
                    width: tabBar.width / 4
                    height: tabBar.height
                    Rectangle {
                        anchors.fill: parent
                        color: ma.pressed ? "#0f3460" : "transparent"
                        Label {
                            anchors.centerIn: parent
                            text: modelData
                            color: "#e0e0e0"
                            font.pixelSize: 12
                        }
                        MouseArea {
                            id: ma
                            anchors.fill: parent
                            onClicked: switchToTab(index)
                        }
                    }
                }
            }
        }
    }

    AlertDialog { id: alertDialog }
}
