#ifndef MAVLINKCLASS_H
#define MAVLINKCLASS_H

#include <QObject>
#include <mavlink/common/mavlink.h>
#include <QUdpSocket>
#include <QHostAddress>
#include <mavlink/mavlink_types.h>
#include <mavlink/mavlink_helpers.h>
#include <mavlink/common/mavlink_msg_rc_channels.h>
#include <mavlink/common/mavlink_msg_rc_channels_override.h>
#include <mavlink/common/mavlink_msg_rc_channels_raw.h>
#include <mavlink/common/mavlink_msg_rc_channels_scaled.h>
#include <mavlink/common/mavlink_msg_command_long.h>


#include "ui_widget.h"
class MavlinkClass : public QObject
{
    Q_OBJECT
public:
    explicit MavlinkClass(QObject *parent = nullptr);
    bool initUdpConnection(quint16 localPort,const  QString &ip, quint16 remotePort);
    void handle_rc_channels(mavlink_message_t &message);
    void sendRCChannelsMessage(quint16 x, quint16 y, quint16 z, quint16 roll, quint16 yaw, quint16 pitch);
    void sendMotorUnlockCommand();
    void sendMotorLockCommand();
public slots:



signals:



private:
    void receiveMessage();  // 用于接收数据的函数
    //mavlink_message_t message;  // 用于存放 mavlink 消息的对象
    QUdpSocket *udpSocket;      // 用于接收 UDP 数据的 socket
    quint8 targetSystem;        // 目标系统 ID
    quint8 targetComponent;     // 目标组件 ID
    QHostAddress targetAddress;
    quint16 targetPort;
};

#endif // MAVLINKCLASS_H
