#ifndef UDPCLASS_H
#define UDPCLASS_H

#include <QObject>
#include <QUdpSocket>
#include <QImage>
#include <QHash>
#include <QByteArray>
#include <QBuffer>
#include <QTimer>
class udpclass : public QObject
{
    Q_OBJECT
public:
    explicit udpclass(QObject *parent = nullptr);
    void sendData(const QByteArray &data, const QString &targetIp, quint16 targetPort); // 发送数据方法
    // 设备类型枚举
    enum DeviceType {
        ROV1,       // 192.168.2.22
        ROV2,       // 192.168.2.16
        Buoy1,      // 192.168.2.19
        Buoy2,       // 192.168.2.25
        Orangepi1,   // 192.168.2.23
        Orangepi2   // 192.168.2.17
    };
signals:
    void dataReceivedFromRobot1(const QByteArray &data);
    void dataReceivedFromRobot2(const QByteArray &data);
    void dataReceivedFromBuoy1(const QByteArray &data);
    void dataReceivedFromBuoy2(const QByteArray &data);
    void dataReceivedFromOrangepi1(const QByteArray &data);
    void dataReceivedFromOrangepi2(const QByteArray &data);
    void dataReceivedFromOther(const QByteArray &data, const QString &senderIp);

private slots:
    void readPendingDatagrams();
    void resetTimedOutBuffers();


private:
    // 组包数据结构
    struct PacketBuffer {
        QByteArray buffer;
        qint64 expectedSize = 0;
        qint64 receivedSize = 0;
        QTimer *timeoutTimer = nullptr; // 超时计时器
    };

    // 设备IP到类型的映射
    const QHash<QString, DeviceType> deviceMap = {
        {"192.168.2.22", ROV1},
        {"192.168.2.16", ROV2},
        {"192.168.2.19", Buoy1},
        {"192.168.2.25", Buoy2},
        {"192.168.2.23", Orangepi1},
        {"192.168.2.19", Orangepi2}
    };

    // 设备独立的组包函数
    void processROV1Buffer(const QByteArray &data);
    void processROV2Buffer(const QByteArray &data);
    void processBuoy1Buffer(const QByteArray &data);
    void processBuoy2Buffer(const QByteArray &data);
    void processOrangepi1(const QByteArray &data);
    void processOrangepi2(const QByteArray &data);
    // 缓冲区管理
    void resetBuffer(DeviceType deviceType);
    void resetAllBuffers();

    QUdpSocket *udpSocket = nullptr;

    // 每个设备独立的缓冲区
    PacketBuffer rov1Buffer;
    PacketBuffer rov2Buffer;
    PacketBuffer buoy1Buffer;
    PacketBuffer buoy2Buffer;
    PacketBuffer Orangepi1Buffer;
    PacketBuffer Orangepi2Buffer;

    // 超时时间 (毫秒)
    const int BUFFER_TIMEOUT = 3000;
};

#endif // UDPCLASS_H
