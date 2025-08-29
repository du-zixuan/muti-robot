#ifndef TRAVERSAL_H
#define TRAVERSAL_H

#include <QObject>
#include <QVector>
#include <QGeoCoordinate>
class traversal : public QObject
{
    Q_OBJECT
public:
    explicit traversal(QObject *parent = nullptr);
    // 生成牛耕式路径 (返回QGeoCoordinate列表)
    QVector<QGeoCoordinate> generateBoustrophedonPath(
        double start_lat, double start_lon,
        double heading, double width, double height,
        double strip_width);

signals:

private:
    QGeoCoordinate localToGPS(double x, double y,
                              double start_lat, double start_lon,
                              double heading);

};

#endif // TRAVERSAL_H
