import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCharts

Page {
    id: chartPage
    background: Rectangle { color: "#1a1a2e" }
    title: "历史图表"

    header: ToolBar {
        background: Rectangle { color: "#16213e" }
        RowLayout {
            anchors.fill: parent; anchors.leftMargin: 12
            ToolButton {
                text: "←"
                onClicked: appWindow.switchToTab(2)
                contentItem: Label { text: "←"; color: "#e0e0e0"; font.pixelSize: 22 }
            }
            Label {
                text: "历史数据"; font.pixelSize: 18; font.bold: true; color: "#e0e0e0"
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 12

        // 传感器选择
        Flow {
            Layout.fillWidth: true
            spacing: 8

            Repeater {
                model: [
                    { type: 10001, label: "CO₂" },
                    { type: 10002, label: "温度" },
                    { type: 10008, label: "湿度" },
                    { type: 10004, label: "PM2.5" },
                    { type: 10005, label: "甲烷" },
                    { type: 10006, label: "光照" },
                    { type: 10007, label: "烟雾" },
                    { type: 10003, label: "火焰" }
                ]
                Button {
                    text: modelData.label
                    checkable: true
                    autoExclusive: true
                    checked: index === 0
                    onClicked: {
                        doQuery(modelData.type, currentRange)
                    }
                    background: Rectangle {
                        radius: 6
                        color: parent.checked ? "#e94560" : "#0f3460"
                    }
                    contentItem: Label {
                        text: parent.text; color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 13
                    }
                }
            }
        }

        // 时间范围选择
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Repeater {
                model: [
                    { range: "1h",  label: "1小时" },
                    { range: "24h", label: "24小时" },
                    { range: "7d",  label: "7天" },
                    { range: "30d", label: "30天" }
                ]
                Button {
                    text: modelData.label
                    checkable: true
                    autoExclusive: true
                    checked: index === 1
                    onClicked: {
                        doQuery(currentSensorType, modelData.range)
                    }
                    background: Rectangle {
                        radius: 6
                        color: parent.checked ? "#0f3460" : "#1a1a2e"
                        border.color: "#0f3460"
                    }
                    contentItem: Label {
                        text: parent.text; color: "#e0e0e0"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.pixelSize: 12
                    }
                }
            }
        }

        // 图表区域
        ChartView {
            id: chartView
            Layout.fillWidth: true
            Layout.fillHeight: true
            antialiasing: true
            backgroundColor: "#16213e"
            legend.visible: true
            legend.color: "#e0e0e0"

            ValueAxis {
                id: axisY
                titleText: ""
                labelFormat: "%.0f"
                color: "#a0a0a0"
                labelsColor: "#a0a0a0"
                gridLineColor: "#2a3a4e"
            }
            DateTimeAxis {
                id: axisX
                format: "MM-dd hh:mm"
                color: "#a0a0a0"
                labelsColor: "#a0a0a0"
                gridLineColor: "#2a3a4e"
            }

            LineSeries {
                id: lineSeries
                name: "传感器数据"
                color: "#e94560"
                width: 2
                axisX: axisX
                axisY: axisY
            }
        }

        // 数据摘要
        Label {
            id: historyLabel
            text: "选择传感器和时间范围查看历史数据"
            color: "#a0a0a0"
            font.pixelSize: 12
            Layout.alignment: Qt.AlignHCenter
        }
    }

    // 连接历史查询结果
    Connections {
        target: controller

        // 收到历史数据（dataJson 为 JSON 字符串）
        function onHistoryReceived(sensorType, range, dataJson) {
            queryTimeout.stop()
            console.log("ChartPage: onHistoryReceived called, sensorType=" + sensorType
                        + " range=" + range + " dataJson=" + dataJson)
            isLoading = false
            lineSeries.clear()

            // 解析 JSON 字符串为 JS 数组
            var data = JSON.parse(dataJson)
            if (!data || data.length === 0) {
                historyLabel.text = "暂无数据"
                console.log("ChartPage: 历史数据为空")
                return
            }

            console.log("ChartPage: 解析到 " + data.length + " 条记录")
            var minVal = Number.MAX_VALUE, maxVal = Number.MIN_VALUE
            var minTime = new Date(), maxTime = new Date(0)

            for (var i = 0; i < data.length; i++) {
                var item = data[i]
                console.log("ChartPage: item[" + i + "] time=" + item.time + " value=" + item.value)
                var t = new Date(item.time.replace(" ", "T"))
                var v = item.value
                lineSeries.append(t.getTime(), v)
                minVal = Math.min(minVal, v)
                maxVal = Math.max(maxVal, v)
                if (t < minTime) minTime = t
                if (t > maxTime) maxTime = t
            }

            axisY.min = minVal * 0.9
            axisY.max = maxVal * 1.1
            axisX.min = minTime
            axisX.max = maxTime

            // 传感器名称映射
            var names = { 10001: "CO₂", 10002: "温度", 10003: "火焰", 10004: "PM2.5",
                          10005: "甲烷", 10006: "光照", 10007: "烟雾", 10008: "湿度" }
            var units = { 10001: "ppm", 10002: "°C", 10003: "", 10004: "µg/m³",
                          10005: "", 10006: "lux", 10007: "", 10008: "%" }
            lineSeries.name = names[sensorType] || "传感器"
            axisY.titleText = units[sensorType] || ""
            historyLabel.text = names[sensorType] + " 历史数据 | " + data.length + " 条记录"
        }

        // 查询失败（未连接等）
        function onHistoryQueryError(msg) {
            isLoading = false
            errorMsg = msg
            historyLabel.text = msg
        }

        // 连接状态恢复时自动重新查询
        function onConnectedChanged() {
            if (controller.connected && hasQueried && errorMsg !== "") {
                doQuery(currentSensorType, currentRange)
            }
        }
    }

    // 初始化：查询默认传感器 24h 数据
    property int currentSensorType: 10001
    property string currentRange: "24h"
    property bool isLoading: false
    property bool hasQueried: false
    property string errorMsg: ""

    // 查询超时计时器（5 秒无响应则提示）
    Timer {
        id: queryTimeout
        interval: 5000
        repeat: false
        onTriggered: {
            if (isLoading) {
                isLoading = false
                errorMsg = "查询超时，服务器未响应"
                historyLabel.text = errorMsg
                console.warn("ChartPage: 历史查询超时，服务器未返回 40002 响应")
            }
        }
    }

    // 统一的查询入口：设置加载状态后发起查询
    function doQuery(sensorType, range) {
        if (!controller.connected) {
            errorMsg = "未连接到服务器，请先连接"
            historyLabel.text = errorMsg
            console.warn("ChartPage: 未连接，跳过查询")
            return
        }
        isLoading = true
        hasQueried = true
        errorMsg = ""
        historyLabel.text = "加载中..."
        currentSensorType = sensorType
        currentRange = range
        console.log("ChartPage: 发送历史查询 sensorType=" + sensorType + " range=" + range)
        queryTimeout.restart()
        controller.queryHistory(sensorType, range)
    }

    Component.onCompleted: {
        doQuery(10001, "24h")
    }
}
