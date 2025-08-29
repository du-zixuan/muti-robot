#include "traversal.h"
#include <QtMath>

// 地球半径(米)
constexpr double EARTH_RADIUS = 6371000.0;
// 纬度每度的米数 (常量)
constexpr double M_PER_DEG_LAT = 111194.9;

traversal::traversal(QObject *parent)
    : QObject{parent}
{}

QVector<QGeoCoordinate> traversal::generateBoustrophedonPath(
    double start_lat, double start_lon,
    double heading, double width, double height,
    double strip_width)
{
    QVector<QGeoCoordinate> waypoints;

    // 计算需要多少条带
    int num_strips = std::ceil(height / strip_width);

    // 添加初始点
    waypoints.append(QGeoCoordinate(start_lat, start_lon));

    // 生成路径点 (在船头坐标系中)
    for (int strip_idx = 0; strip_idx < num_strips; ++strip_idx) {
        double y = strip_idx * strip_width + strip_width / 2.0;

        if (strip_idx % 2 == 0) { // 偶数条带：左→右
            // 起点 (0, y)
            waypoints.append(localToGPS(0, y, start_lat, start_lon, heading));
            // 终点 (width, y)
            waypoints.append(localToGPS(width, y, start_lat, start_lon, heading));
        } else { // 奇数条带：右→左
            // 起点 (width, y)
            waypoints.append(localToGPS(width, y, start_lat, start_lon, heading));
            // 终点 (0, y)
            waypoints.append(localToGPS(0, y, start_lat, start_lon, heading));
        }
    }

    return waypoints;
}

QGeoCoordinate traversal::localToGPS(double x, double y,
                                     double start_lat, double start_lon,
                                     double heading)
{
    // 计算经度每度的米数 (依赖于纬度)
    double m_per_deg_lon = M_PER_DEG_LAT * std::cos(qDegreesToRadians(start_lat));

    // 坐标旋转（船头坐标系转地理坐标系）
    double theta = qDegreesToRadians(heading);
    double east = x * std::sin(theta) + y * std::cos(theta);
    double north = x * std::cos(theta) - y * std::sin(theta);

    // 转换为GPS坐标
    double lat = start_lat + north / M_PER_DEG_LAT;
    double lon = start_lon + east / m_per_deg_lon;

    return QGeoCoordinate(lat, lon);
}
