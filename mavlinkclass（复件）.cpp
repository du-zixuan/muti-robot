#include "mavlinkclass.h"

MavlinkClass::MavlinkClass(QObject *parent)
    : QObject{parent}
{
    // 创建 MAVLink 连接
    udpSocket = new QUdpSocket(this);
}
// 初始化 UDP 连接
bool MavlinkClass::initUdpConnection(quint16 localPort,const  QString &ip, quint16 remotePort)
{
    // 绑定本地端口
    if (!udpSocket->bind(QHostAddress::Any, localPort)) {
        qDebug() << "bind error: " << strerror(errno);
        return false;
    }

    // 目标地址和端口
    QHostAddress targetAddress(ip);
    quint16 targetPort = remotePort;

    // 启动接收消息的信号
    connect(udpSocket, &QUdpSocket::readyRead, this, &MavlinkClass::receiveMessage);

    qDebug() << "MAVLink connection established.";
    return true;
}


void MavlinkClass::receiveMessage()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        mavlink_message_t message_rec;
        mavlink_status_t status;

        // 逐字节解析数据流
        for (int i = 0; i < datagram.size(); ++i) {
            if (mavlink_parse_char(MAVLINK_COMM_0, datagram[i], &message_rec, &status)) {
                // 处理 RC_CHANNELS 消息
                if (message_rec.msgid == MAVLINK_MSG_ID_RC_CHANNELS) {
                    handle_rc_channels(message_rec);
                }
            }
        }
    }
}
void MavlinkClass::handle_rc_channels(mavlink_message_t &message_rec)
{
    mavlink_rc_channels_t rc_channels;
    mavlink_msg_rc_channels_decode(&message_rec, &rc_channels);

    qDebug() << "Received RC Channels:";
    qDebug() << "Roll:" << rc_channels.chan1_raw;
    qDebug() << "Pitch:" << rc_channels.chan2_raw;
    qDebug() << "Throttle:" << rc_channels.chan3_raw;
    qDebug() << "Yaw:" << rc_channels.chan4_raw;
}

void MavlinkClass::sendRCChannelsMessage(quint16 x, quint16 y, quint16 z, quint16 roll, quint16 yaw, quint16 pitch)
{
    // 初始化消息结构体
    mavlink_message_t message;

    // 调用 mavlink_msg_rc_channels_override_pack 填充消息数据
    mavlink_msg_rc_channels_override_pack(
        1,   // 系统 ID
        200, // 组件 ID
        &message,           // 消息结构体指针
        1,                   // 目标系统 ID
        200,                 // 目标组件 ID
        roll,                // 通道 1 原始值
        pitch,               // 通道 2 原始值
        z,                   // 通道 3 原始值
        yaw,                 // 通道 4 原始值
        x,                   // 通道 5 原始值
        y,                   // 通道 6 原始值
        1500,                // 通道 7 原始值（通常为 1500）
        1500,                // 通道 8 原始值（通常为 1500）
        1100,                // 通道 9 原始值
        1100,                // 通道 10 原始值
        1100,                // 通道 11 原始值
        0,                // 通道 12 原始值
        0,                // 通道 13 原始值
        0,                // 通道 14 原始值
        0,                // 通道 15 原始值
        0,                // 通道 16 原始值
        0,                // 通道 17 原始值
        0                 // 通道 18 原始值
        );


    // 创建一个缓冲区用于发送数据
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

    // 获取消息的长度并将消息转换为字节数组
    int len = mavlink_msg_to_send_buffer(buffer, &message);

    // 通过 QUdpSocket 发送消息
    udpSocket->writeDatagram(reinterpret_cast<char*>(buffer),  targetAddress, targetPort);

    //qDebug()<<"ok1";
}
void MavlinkClass::sendMotorUnlockCommand()
{
    // 初始化消息结构体
    mavlink_message_t message;

    // 系统 ID 和 组件 ID 可以按需要设置
    uint8_t system_id = 1;  // 发送命令的系统 ID
    uint8_t component_id = 200;  // 发送命令的组件 ID
    uint8_t target_system_id = 1;  // 目标系统 ID
    uint8_t target_component_id = 200;  // 目标组件 ID

    uint16_t command = MAV_CMD_COMPONENT_ARM_DISARM; // 解锁电机命令
    uint8_t confirmation = 1;  // 通常确认（1: 命令确认，0: 不确认）

    // 通过 mavlink_msg_command_long_pack 填充消息数据
    mavlink_msg_command_long_pack(
        system_id,            // 系统 ID
        component_id,         // 组件 ID
        &message,             // 消息结构体指针
        target_system_id,     // 目标系统 ID
        target_component_id,  // 目标组件 ID
        command,              // 命令类型，解锁电机
        confirmation,         // 确认标志
        1.0f,                 // param1: 1.0表示解锁电机，0.0表示锁定电机
        0.0f,                 // param2: 不使用
        0.0f,                 // param3: 不使用
        0.0f,                 // param4: 不使用
        0.0f,                 // param5: 不使用
        0.0f,                 // param6: 不使用
        0.0f                  // param7: 不使用
        );
    qDebug() << "Message sent successfully3";

    // 创建一个缓冲区用于发送数据
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    qDebug() << "Message sent successfully2";

    // 获取消息的长度并将消息转换为字节数组
    int len = mavlink_msg_to_send_buffer(buffer, &message);
    qDebug() << "Message sent successfully1";

    // 通过 QUdpSocket 发送消息
    udpSocket->writeDatagram(reinterpret_cast<char*>(buffer),  targetAddress, targetPort);
    qDebug() << "Message sent successfully";

}
void MavlinkClass::sendMotorLockCommand()
{
    // 初始化消息结构体
    mavlink_message_t message;

    // 系统 ID 和 组件 ID 可以按需要设置
    uint8_t system_id = 1;  // 发送命令的系统 ID
    uint8_t component_id = 200;  // 发送命令的组件 ID
    uint8_t target_system_id = 1;  // 目标系统 ID
    uint8_t target_component_id = 200;  // 目标组件 ID

    uint16_t command = MAV_CMD_COMPONENT_ARM_DISARM; // 解锁电机命令
    uint8_t confirmation = 1;  // 通常确认（1: 命令确认，0: 不确认）

    // 通过 mavlink_msg_command_long_pack 填充消息数据
    mavlink_msg_command_long_pack(
        system_id,            // 系统 ID
        component_id,         // 组件 ID
        &message,             // 消息结构体指针
        target_system_id,     // 目标系统 ID
        target_component_id,  // 目标组件 ID
        command,              // 命令类型，解锁电机
        confirmation,         // 确认标志
        0.0f,                 // param1: 0.0表示上锁电机
        0.0f,                 // param2: 不使用
        0.0f,                 // param3: 不使用
        0.0f,                 // param4: 不使用
        0.0f,                 // param5: 不使用
        0.0f,                 // param6: 不使用
        0.0f                  // param7: 不使用
        );

    // 创建一个缓冲区用于发送数据
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

    // 获取消息的长度并将消息转换为字节数组
    int len = mavlink_msg_to_send_buffer(buffer, &message);

    // 通过 QUdpSocket 发送消息
    udpSocket->writeDatagram(reinterpret_cast<char*>(buffer), len, targetAddress, targetPort);
}
