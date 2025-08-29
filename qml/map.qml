import QtQuick 2.12
//import QtQuick 2.15
import QtLocation 5.15
import QtPositioning 5.15

Map {
    id: map
    plugin: Plugin {
        name: "osm"
        PluginParameter { name: "osm.mapping.providersrepository.address"; value: "http://maps-redirect.qt.io/osm/5.8/" }
    }
    center: QtPositioning.coordinate(39.9042, 116.4074) // 初始中心点（北京）
    zoomLevel: 10

    // 鼠标点击事件
    MouseArea {
        anchors.fill: parent
        onClicked: {
            var coord = map.toCoordinate(Qt.point(mouse.x, mouse.y));
            console.log("Clicked at:", coord.latitude, coord.longitude);
            // 发送信号到C++
            mapHandler.addWaypoint(coord.latitude, coord.longitude);
        }
    }
}
