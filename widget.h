#ifndef WIDGET_H
#define WIDGET_H
#define MAVLINK_USE_OLD_PROTOCOL 1
#include <QWidget>
#include <QGamepad>
#include <QGamepadManager>
#include<QTimer>
#include <QThread>
#include <QLineSeries>
#include <QChartView>
#include <QValueAxis>
#include <QChart>
#include <mavlink/common/mavlink.h>
#include <mavlinkclass.h>
#include <VirtualKeyboard.h>
#include <QtLocation/QGeoServiceProvider>
#include <QtPositioning/QGeoCoordinate>
#include <QGraphicsView>
#include <QGeoPositionInfoSource>
#include <QGeoServiceProvider>
#include <QGeoCoordinate>
#include <QQuickItem>
#include <QVector>
#include <QtPositioning/QGeoCoordinate>
#include <QQmlContext>
#include <QVBoxLayout>
#include <QGamepad>
#include <QGamepadManager>
#include "udpclass.h"
#include <QGraphicsDropShadowEffect>
#include <QPainter> // 用于抗锯齿设置
#include <traversal.h>
#include <udpvideo.h>
#include <tracking.h>
QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_CHARTS_USE_NAMESPACE
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void Widget_init();
    ~Widget();
    //void virtualKeyboard_robot1_connect();
    //void virtualKeyboard_robot2_connect();
    //void init_mavlink();
    void robot1_connect();
    void sendPathToROV1();

public slots:
    //void handleUdpData(const QByteArray &data); // 处理接收到的UDP数据
    // 添加两个新的槽函数
    void handleUdpData1(const QByteArray &data);  // 处理来自机器人1的数据
    void handleUdpData2(const QByteArray &data);  // 处理来自机器人2的数据
    void handleBuoy1Data(const QByteArray &data);
    void handleBuoy2Data(const QByteArray &data);
    void handleOtherUdpData(const QByteArray &data, const QString &senderIp); // 处理其他来源数据
    //手柄控制
    //x和y平动控制
    void shoubing_pingdong_x(double value);
    void shoubing_pingdong_y(double value);
    void shoubing_pingdong_z(double value);
    void init_sign_slots();
    //z平动控制即上升下潜
    //俯仰（pitch）和左右（yaw）侧翻(roll)控制
    void shoubing_bulletmove(double value);
    void shoubing_roll_open(double value);
    void shoubing_zhuandong_yaw(double value);
    void shoubing_free_move(double value);
    void shoubing_dingshen(double value);
    void onrobot1_connect();
    void onrobot2_connect();
    void onRadioButtonxyz();
    void onRadioButtonpage();
    void unpack_robot1_zero(QByteArray data_hear,int head);
    void unpack_robot2_zero(QByteArray data_hear,int head);
    void unpack_Buoy1_zero(QByteArray data_hear,int head);
    void unpack_Buoy2_zero(QByteArray data_hear,int head);
    //机器人电机上锁
    void robot1_locak(double value);
    //void robot1_start_connect();
    //void robot1_data_transform();
    void print_move_data();
    //void robot1_unlocak();
    //void robot1_locak();
    void addWaypoint(double lat, double lon);
    //void robot1_heartbeat_send();
    void traversal_start();
    void visualizePathOnMap();
    // ... 其他槽函数 ...
    void updateCameraView(QImage leftImage, const QString &senderIp);
    void updateDepthInfo(float depth, const QString &senderIp);
    //void handleFloatFeedback(const QVector<floaupdateCameraViewt> &feedback, const QString &senderIp);
    // 添加槽函数
    void on_pushButton_track_clicked();
    void handleTrackingCommand(float yawSpeed, float moveSpeed);
    void updateTrackingVisual(QImage processedImage);
    // ... 其他槽函数 ...
    void updateTargetPosition(quint16 x, quint16 y, const QString &senderIp);
private:
    Ui::Widget *ui;
    QGamepad* m_gamepad;
    QThread *u1,*tr1,*v1, *v2; // 添加v2线程
    class udpclass *udp;
    class traversal *pathPlanner;
    class udpvideo *udp_video1; // 第一个ROV视频
    class udpvideo *udp_video2; // 第二个ROV视频
    /* 图表对象 */
    QChart* m_chart_all;
    QValueAxis *m_axisX, *m_axisY;
    QLineSeries* m_lineSeries_X;
    QLineSeries* m_lineSeries_Y;
    QLineSeries* m_lineSeries_Z;
    /* 横纵坐标最大显示范围 */
    const int AXIS_MAX_X = 20, AXIS_MAX_Y = 400, AXIS_MIN_Y = -400;
    /* 用于模拟生成实时数据的定时器 */
    QTimer * timer_for_move;
    QTimer* m_timer;
    QTimer*m_timer2;
    int pointCount = 0;
    int pointCount2 = 0;
    float plot_x;
    float plot_y;
    float plot_z;
    /* 曲线图对象 */
    QLineSeries* m_lineSeries_roll;
    QLineSeries* m_lineSeries_pitch;
    QLineSeries* m_lineSeries_yaw;
    /* 横纵坐标轴对象 */
    QChart* m_chart_alljiaodu;
    QValueAxis *m_axisX2, *m_axisY2, *m_axisY_roll;
    QValueAxis *m_axisY_pitch;
    QValueAxis *m_axisY_yaw;
    /* 横纵坐标最大显示范围 */
    const int AXIS_MAX_Y_roll = 10, AXIS_MIN_Y_roll = -5;
    const int AXIS_MAX_Y_pitch = 10,AXIS_MIN_Y_pitch = -5;
    const int AXIS_MAX_Y_yaw = 180,AXIS_MIN_Y_yaw = -180;
    const int AXIS_MAX_Y_shendu = 0.6;
    float plot_roll;
    float plot_yaw;
    float plot_pitch;
    VirtualKeyboard *virtualKeyboard;  // 虚拟键盘对象
    QThread *mavlinkThread1,*mavlinkThread2;
    class MavlinkClass *mavlinkClass1;
    class MavlinkClass *mavlinkClass2;
    bool robot1_connect_flag=true;
    QTimer * robot1_timer_push,* robot2_timer_push,*robot1_heartbeat_timer_push;
    QVector<QGeoCoordinate> waypoints; // 存储经纬度的容器
    uint8_t is_button=6;
    uint8_t is_connect=0;
    float water_state[18];
    uint16_t pwm[8];
    float angle_data[3];
    float GPS_Buoy1_state[2];
    float GPS_Buoy2_state[2];
    QVector<QGeoCoordinate> waypoint_ROV1; // ROV1的路径点 (包含初始点和目标点)
    double rov1_heading = 45.0; // ROV1船头方向 (度，正北为0)
    const double SEARCH_WIDTH = 6.0; // 搜索区域宽度 (米)
    const double SEARCH_HEIGHT = 6.0; // 搜索区域高度 (米)
    const double STRIP_WIDTH = 2.0; // 条带宽度 (米)
    qint32 longitude_ROV1;
    qint32 latitude_ROV1;
    // ... 其他成员 ...
    double focalLength = 1000.0; // 默认焦距值
    double baseline = 0.12;      // 默认基线值
    QHash<QString, float> depthInfoMap;       // 存储深度信息
    QHash<QString, QVector<float>> floatFeedbackMap; // 存储浮漂坐标信息
    // 添加成员变量
    Tracking *m_tracking;
    QThread *m_trackingThread;

};
#endif // WIDGET_H
