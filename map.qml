import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Map {
    id: map
    plugin: Plugin {
        name: "osm"
        PluginParameter { name: "osm.mapping.providersrepository.address"; value: "http://maps-redirect.qt.io/osm/5.8/" }
    }
    center: QtPositioning.coordinate(39.9042, 116.4074) // 初始中心点（北京）
    zoomLevel: 12

    // 定义 ListModel 来存储路标数据
    ListModel {
        id: waypointModel
    }

    // 存储红线的经纬度
    ListModel {
        id: polylineModel
    }

    // 鼠标点击事件
    MouseArea {
        anchors.fill: parent
        onClicked: {
            var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
            console.log("Clicked at:", coord.latitude, coord.longitude);

            // 向 waypointModel 中添加新的路标
            waypointModel.append({
                latitude: coord.latitude,
                longitude: coord.longitude
            });

            // 向 polylineModel 中添加新的坐标点
            polylineModel.append({
                latitude: coord.latitude,
                longitude: coord.longitude
            });

            // 发送信号到C++，将点击的经纬度信息传递给 C++ 层
            mapHandler.addWaypoint(coord.latitude, coord.longitude);

            // 更新 MapPolyline 的 path
            updatePolylinePath();
        }
    }

    // 显示路标
    MapItemView {
        model: waypointModel
        delegate: MapQuickItem {
            coordinate: QtPositioning.coordinate(model.latitude, model.longitude)
            anchorPoint.x: redDot.width / 2  // 确保红点居中
            anchorPoint.y: redDot.height / 2

            // 使用 Rectangle 绘制一个红色小圆点作为路标
            sourceItem: Rectangle {
                id: redDot
                width: 12
                height: 12
                radius: 6  // 使矩形变为圆形
                color: "red"  // 红色
            }
        }
    }

    // 绘制红色连接线
    MapPolyline {
        id: polyline
        width: 2
        line.color: "red"

        // 绑定 path 到 polylineModel，动态更新折线路径
        path: polylineModel.path
    }

    // 更新 polyline 路径
    function updatePolylinePath() {
        var path = [];
        for (var i = 0; i < polylineModel.count; i++) {
            var waypoint = polylineModel.get(i);
            // 将每个坐标点转换为 QGeoCoordinate 对象，并添加到 path 数组中
            path.push(QtPositioning.coordinate(waypoint.latitude, waypoint.longitude));
        }
        polyline.path = path;  // 更新 MapPolyline 的路径
    }
}




