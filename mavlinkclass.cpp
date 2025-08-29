#include "mavlinkclass.h"

MavlinkClass::MavlinkClass(QObject *parent)
    : QObject{parent}, serialPort(nullptr)   // 先不初始化 QSerialPort
{
    // 创建 MAVLink 连接
    udpSocket = new QUdpSocket(this);
    // 创建串口对象
    serialPort = new QSerialPort(this);
}
// 初始化 UDP 连接
bool MavlinkClass::initUdpConnection(const  QString &ip, quint16 remotePort)
{
    // 使用 QHostAddress 来绑定 IP 地址
    QHostAddress localAddress("192.168.2.13");
    //qDebug()<<"ok1";
     //绑定本地端口
    if (!udpSocket->bind(QHostAddress::Any, 14552)) {
        qDebug() << "bind error: " << strerror(errno);
        qDebug()<<"ok2";
        return false;
    }
   // qDebug()<<"ok3";
    // 目标地址和端口
    targetAddress = QHostAddress("192.168.2.13");  // 树莓派的 IP 地址
    targetPort = 14551;  // 或者 14551
    //()<<"targetPort:"<<targetPort;
    // 启动接收消息的信号
    connect(udpSocket, &QUdpSocket::readyRead, this, &MavlinkClass::receiveMessage);

    qDebug() << "MAVLink connection established.";
    return true;
}

bool MavlinkClass::initSerialConnection(const QString &portName, qint32 baudRate)
{
    serialPort->setPortName(portName);
    serialPort->setBaudRate(baudRate);
    serialPort->setDataBits(QSerialPort::Data8);
    serialPort->setParity(QSerialPort::NoParity);
    serialPort->setStopBits(QSerialPort::OneStop);
    serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort->open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open serial port:" << serialPort->errorString();
        return false;
    }

    // 启动接收消息的信号
    connect(serialPort, &QSerialPort::readyRead, this, &MavlinkClass::receiveMessage_serial);

    qDebug() << "MAVLink serial connection established.";
    return true;
}
void MavlinkClass::receiveMessage()
{
    //qDebug()<<"被调用了";
    while (udpSocket->hasPendingDatagrams()) {
        //qDebug()<<"读取成功了";
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());

        mavlink_message_t message_rec;
        mavlink_status_t status;

        // 逐字节解析数据流
        for (int i = 0; i < datagram.size(); ++i) {
            uint8_t byte = static_cast<uint8_t>(datagram[i]);
        //qDebug()<<"开始解析了";
            // 调用 mavlink_parse_char
            if (mavlink_parse_char(MAVLINK_COMM_0, byte, &message_rec, &status)) {
                // 打印接收到的消息的 msgid
                //qDebug() << "Received MAVLink message with msgid:" << message_rec.msgid;

                // 根据 msgid 处理不同的消息类型
                switch (message_rec.msgid) {
                case MAVLINK_MSG_ID_RC_CHANNELS:
                    handle_rc_channels(message_rec);
                    break;
                case MAVLINK_MSG_ID_HEARTBEAT:
                    handle_heartbeat(message_rec);
                    break;
                case MAVLINK_MSG_ID_COMMAND_LONG:
                   // handle_command_long(message_rec);
                    qDebug()<<"haha";
                   break;

                case MAVLINK_MSG_ID_COMMAND_ACK:
                    qDebug()<<"haha";
                    break;
                // 可以在此添加更多的消息处理
                case MAVLINK_MSG_ID_AUTOPILOT_VERSION: {
                    // 打印 AUTOPILOT_VERSION 消息内容
                    mavlink_autopilot_version_t autopilot_version;
                    mavlink_msg_autopilot_version_decode(&message_rec, &autopilot_version);

                    qDebug() << "Received AUTOPILOT_VERSION:";
                    qDebug() << "Capabilities: " << autopilot_version.capabilities;
                    qDebug() << "Flight SW version: " << autopilot_version.flight_sw_version;
                    qDebug() << "Middleware SW version: " << autopilot_version.middleware_sw_version;
                    qDebug() << "OS SW version: " << autopilot_version.os_sw_version;
                    qDebug() << "Board version: " << autopilot_version.board_version;
                    qDebug() << "Vendor ID: " << autopilot_version.vendor_id;
                    qDebug() << "Product ID: " << autopilot_version.product_id;
                    qDebug() << "UID: " << autopilot_version.uid;
                    break; }
                default:
                    // 未处理的消息类型
                    break;
                }
            }
        }
    }
}

void MavlinkClass::receiveMessage_serial()
{
    QByteArray data = serialPort->readAll(); // 通过串口读取数据
    mavlink_message_t message_rec;
    mavlink_status_t status;

    // 逐字节解析数据流
    for (char byte : data) {
        uint8_t parsedByte = static_cast<uint8_t>(byte);

        // 调用 mavlink_parse_char
        if (mavlink_parse_char(MAVLINK_COMM_0, parsedByte, &message_rec, &status)) {
            // 处理 RC_CHANNELS 消息
            if (message_rec.msgid == MAVLINK_MSG_ID_RC_CHANNELS) {
                handle_rc_channels(message_rec);
            }
            // 检查是否是 HEARTBEAT 消息
            else if (message_rec.msgid == MAVLINK_MSG_ID_HEARTBEAT) {
                handle_heartbeat(message_rec);
            }
            // 处理 COMMAND_LONG 消息
            else if (message_rec.msgid == MAVLINK_MSG_ID_COMMAND_LONG) {
                handle_heartbeat(message_rec);
            }
            else if (message_rec.msgid == MAVLINK_MSG_ID_COMMAND_ACK) {
                qDebug()<<"h1h1h1h1hh1h1h1h1h1hh1h1h1";
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
    // 打包消息
    mavlink_msg_rc_channels_override_pack(
        1,   // 系统 ID
        200, // 组件 ID
        &message,           // 消息结构体指针
        1,                   // 目标系统 ID
        1,                   // 目标组件 ID
        x,                // 通道 1 原始值
        y,               // 通道 2 原始值
        z,                   // 通道 3 原始值
        roll,                 // 通道 4 原始值
        yaw,                   // 通道 5 原始值
        pitch,                   // 通道 6 原始值
        yaw,                // 通道 7 原始值（中心值）
        yaw               // 通道 8 原始值（中心值）
        );


    // 创建一个缓冲区用于发送数据
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    // 获取消息的长度并将消息转换为字节数组
    int len = mavlink_msg_to_send_buffer(buffer, &message);
    //udp发送数据
    // 将缓冲区转换为 QByteArray，以便通过 QUdpSocket 发送
    QByteArray datagram(reinterpret_cast<char*>(buffer), len);
    udpSocket->writeDatagram(datagram, targetAddress, targetPort);
    //qDebug()<<"targetPort:"<<targetPort;
    // 通过串口发送数据
    //serialPort->write(reinterpret_cast<char*>(buffer), len);
}
void MavlinkClass::sendMotorUnlockCommand()
{
    // 初始化消息结构体
    mavlink_message_t message;

    // 系统 ID 和 组件 ID
    uint8_t system_id = 1;           // 发送命令的系统 ID
    uint8_t component_id = 200;      // 发送命令的组件 ID
    uint8_t target_system_id = 1;    // 目标系统 ID
    uint8_t target_component_id = 1; // 目标组件 ID (通常为 autopilot）

    uint16_t command = MAV_CMD_COMPONENT_ARM_DISARM;  // 解锁电机命令
    uint8_t confirmation = 0;        // 0 = 解锁电机，1 = 锁定电机

    // MAV_CMD_COMPONENT_ARM_DISARM 需要使用正确的 param1 来指定
    // 解锁电机时 param1 设置为 1，锁定电机时 param1 设置为 0
    float param1 = 1.0f;  // 1.0 表示解锁电机

    // 使用 mavlink_msg_command_long_pack 填充消息数据
    mavlink_msg_command_long_pack(
        system_id,            // 系统 ID
        component_id,         // 组件 ID
        &message,             // 消息结构体指针
        target_system_id,     // 目标系统 ID
        target_component_id,  // 目标组件 ID
        command,              // 命令类型，解锁电机
        confirmation,         // 确认标志，0 表示解锁
        param1,               // param1: 1.0 表示解锁电机
        0.0f,                 // param2: 不使用
        0.0f,                 // param3: 不使用
        0.0f,                 // param4: 不使用
        0.0f,                 // param5: 不使用
        0.0f,                 // param6: 不使用
        0.0f                  // param7: 不使用
        );

    qDebug() << "Motor Unlock Command Prepared";
    // 创建一个缓冲区用于发送数据
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

    // 获取消息的长度并将消息转换为字节数组
    int len = mavlink_msg_to_send_buffer(buffer, &message);
    QByteArray datagram(reinterpret_cast<char*>(buffer), len);
    udpSocket->writeDatagram(datagram, targetAddress, targetPort);
    qDebug() << "Motor unlock command sent successfully";

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

    uint16_t command = MAV_CMD_COMPONENT_ARM_DISARM; // 上锁电机命令
    uint8_t confirmation = 1;  // 1: 命令确认

    // 通过 mavlink_msg_command_long_pack 填充消息数据
    mavlink_msg_command_long_pack(
        system_id,
        component_id,
        &message,
        target_system_id,
        target_component_id,
        command,
        confirmation,
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f
        );

    // 创建一个缓冲区用于发送数据
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];
    int len = mavlink_msg_to_send_buffer(buffer, &message);
    //udp发送数据
    // 将缓冲区转换为 QByteArray，以便通过 QUdpSocket 发送
    QByteArray datagram(reinterpret_cast<char*>(buffer), len);
    //int len = mavlink_msg_to_send_buffer(buffer, &message);

    // 通过串口发送数据
    //serialPort->write(reinterpret_cast<char*>(buffer), len);
    // 通过 QUdpSocket 发送消息
    udpSocket->writeDatagram(datagram, targetAddress, targetPort);
    // 通过串口发送数据
    //serialPort->write(reinterpret_cast<char*>(buffer), len);
    //serialPort->write(reinterpret_cast<char*>(buffer), len);
    //qDebug() << "Motor unlock command sent successfully";
    qDebug() << "Motor lock command sent successfully";
}

void MavlinkClass::send_heartbeat()
{
    mavlink_message_t msg;
    uint8_t buffer[MAVLINK_MAX_PACKET_LEN];

    // 设置 HEARTBEAT 参数
    uint8_t type = MAV_TYPE_QUADROTOR;              // 设备类型（四旋翼）
    uint8_t autopilot = MAV_AUTOPILOT_GENERIC;      // 通用自动驾驶
    uint8_t base_mode = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED | MAV_MODE_FLAG_SAFETY_ARMED; // 启用自定义模式 & 使能解锁
    uint32_t custom_mode = 0;                       // 手动模式: 设为 0，部分固件可能需要 `MAV_MODE_MANUAL_ARMED`
    uint8_t system_status = MAV_STATE_ACTIVE;       // 飞行器状态：活跃（ACTIVE）

    // 使用 mavlink_msg_heartbeat_pack() 打包 HEARTBEAT 消息
    mavlink_msg_heartbeat_pack(1, 200, &msg, 12, 3, 209, 19, 5);

    // 将 MAVLink 消息转换为二进制数据流
    int len = mavlink_msg_to_send_buffer(buffer, &msg);
    QByteArray datagram(reinterpret_cast<char*>(buffer), len);
    udpSocket->writeDatagram(datagram, targetAddress, targetPort);
    qDebug()<<"心跳包发送了";
}

void MavlinkClass::handle_heartbeat(mavlink_message_t &message_rec)
{
    mavlink_heartbeat_t heartbeat;
    mavlink_msg_heartbeat_decode(&message_rec, &heartbeat);

    // 检查 HEARTBEAT 消息中的 base_mode 是否包含解锁标志
    if (heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED) {
        qDebug() << "Motors are armed (unlocked).";
    } else {
        //qDebug() << "Motors are disarmed (locked).";
    }
}


