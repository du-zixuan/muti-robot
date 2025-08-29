#ifndef MAVLINKCLASS_H
#define MAVLINKCLASS_H
#define MAVLINK_USE_OLD_PROTOCOL 1
#include <QObject>
#include <mavlink/common/mavlink.h>
#include <QUdpSocket>
#include <QHostAddress>
#include <QSerialPort>
#include <mavlink/mavlink_types.h>
#include <mavlink/mavlink_helpers.h>
#include <mavlink/common/mavlink_msg_rc_channels.h>
#include <mavlink/common/mavlink_msg_rc_channels_override.h>
#include <mavlink/common/mavlink_msg_rc_channels_raw.h>
#include <mavlink/common/mavlink_msg_rc_channels_scaled.h>
#include <mavlink/common/mavlink_msg_command_long.h>
#include <mavlink/common/mavlink_msg_manual_control.h>
#include <QGraphicsView>
#include <QGeoPositionInfoSource>
#include <QGeoServiceProvider>
#include <QGeoCoordinate>
#include <QQuickItem>
#include <QVector>
#include <QtPositioning/QGeoCoordinate>
#include "ui_widget.h"
class MavlinkClass : public QObject
{
    Q_OBJECT
public:
    explicit MavlinkClass(QObject *parent = nullptr);
    bool initUdpConnection(const  QString &ip, quint16 remotePort);
    bool initSerialConnection(const QString &portName, qint32 baudRate);  // 初始化串口连接
    void handle_rc_channels(mavlink_message_t &message);
    void sendRCChannelsMessage(quint16 x, quint16 y, quint16 z, quint16 roll, quint16 yaw, quint16 pitch);
    void sendMotorUnlockCommand();
    void sendMotorLockCommand();
    void handle_heartbeat(mavlink_message_t &message);
    void send_heartbeat();


public slots:

void receiveMessage();  // 用于接收数据的函数
void receiveMessage_serial();  // 用于接收数据的函数

signals:



private:
    //void receiveMessage();  // 用于接收数据的函数
    //mavlink_message_t message;  // 用于存放 mavlink 消息的对象
    QUdpSocket *udpSocket;      // 用于接收 UDP 数据的 socket
    QSerialPort *serialPort;  // 用于串口通信的对象
    quint8 targetSystem;        // 目标系统 ID
    quint8 targetComponent;     // 目标组件 ID
    QHostAddress targetAddress;
    quint16 targetPort;
};

#endif // MAVLINKCLASS_H
