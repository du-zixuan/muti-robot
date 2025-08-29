#include "udpclass.h"
#include <QDebug>

udpclass::udpclass(QObject *parent)
    : QObject{parent}
{
    udpSocket = new QUdpSocket(this);
     udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 1024 * 1024);
    // 绑定到特定 IP 地址 192.168.2.13，端口 14550
    QHostAddress bindAddress("192.168.2.13");

    if (!udpSocket->bind(bindAddress, 7700, QUdpSocket::ShareAddress)) {
        qDebug() << "Failed to bind UDP socket to" << bindAddress.toString() << ":"
                 << udpSocket->errorString();
    } else {
        qDebug() << "UDP socket bound to" << bindAddress.toString() << ":7700";
        connect(udpSocket, &QUdpSocket::readyRead, this, &udpclass::readPendingDatagrams);
    }
    // 设置缓冲区超时计时器
    QTimer *timeoutTimer = new QTimer(this);
    connect(timeoutTimer, &QTimer::timeout, this, &udpclass::resetTimedOutBuffers);
    timeoutTimer->start(1000); // 每秒检查一次
}

void udpclass::sendData(const QByteArray &data, const QString &targetIp, quint16 targetPort)
{
    if (udpSocket) {
        QHostAddress targetAddress(targetIp);
        if (targetAddress.isNull()) {
            qDebug() << "Invalid IP address:" << targetIp;
            return;
        }

        qint64 sent = udpSocket->writeDatagram(data, targetAddress, targetPort);
        if (sent < 0) {
            qDebug() << "Failed to send data:" << udpSocket->errorString();
        } else {
            qDebug() << "Sent" << sent << "bytes to" << targetIp << ":" << targetPort;
        }
    }
}

void udpclass::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        qint64 bytesRead = udpSocket->readDatagram(
            datagram.data(),
            datagram.size(),
            &sender,
            &senderPort
            );

        if (bytesRead <= 0) continue;

        QString senderIp = sender.toString();

        // 根据IP地址路由到不同的处理函数
        if (deviceMap.contains(senderIp)) {
            DeviceType deviceType = deviceMap[senderIp];
            switch (deviceType) {
            case ROV1:
                processROV1Buffer(datagram);
                break;
            case ROV2:
                processROV2Buffer(datagram);
                break;
            case Buoy1:
                processBuoy1Buffer(datagram);
                break;
            case Buoy2:
                processBuoy2Buffer(datagram);
                break;
            case Orangepi1:
                processOrangepi1(datagram);
            case Orangepi2:
                processOrangepi2(datagram);
            }
        } else {
            qDebug() << "Received data from unknown device:" << senderIp;
            emit dataReceivedFromOther(datagram, senderIp);
        }
    }
}

// ROV1 (192.168.2.22) 组包处理
void udpclass::processROV1Buffer(const QByteArray &data)
{
    PacketBuffer &pb = rov1Buffer;

    // 启动/重置超时计时器
    if (pb.timeoutTimer) {
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    } else {
        pb.timeoutTimer = new QTimer(this);
        pb.timeoutTimer->setSingleShot(true);
        connect(pb.timeoutTimer, &QTimer::timeout, [this]() {
            qDebug() << "ROV1 buffer timeout, resetting";
            resetBuffer(ROV1);
        });
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    }

    // 如果是新数据包，查找帧头
    if (pb.expectedSize == 0) {
        int startIndex = data.indexOf('\xFE');
        if (startIndex == -1) {
            qDebug() << "No packet header found for ROV1";
            return;
        }

        // 跳过帧头前的无效数据
        pb.buffer = data.mid(startIndex);
        pb.receivedSize = pb.buffer.size();

        // 验证载荷长度
        if (pb.receivedSize >= 3) {
            uchar payloadLen = static_cast<uchar>(pb.buffer[2]);
            pb.expectedSize = 1 + 1 + 1 + payloadLen + 1 + 1;
            qDebug() << "New ROV1 packet started | Expected size:" << pb.expectedSize;
        }
    } else {
        // 追加到现有缓冲区
        pb.buffer.append(data);
        pb.receivedSize += data.size();
    }

    // 检查是否收到完整数据包
    if (pb.expectedSize > 0 && pb.receivedSize >= pb.expectedSize) {
        // 提取完整数据包
        QByteArray fullPacket = pb.buffer.left(pb.expectedSize);

        // 验证帧尾
        if (static_cast<uchar>(fullPacket[fullPacket.size() - 1]) == 0xFF) {
            qDebug() << "Complete ROV1 packet received | Size:" << fullPacket.size();
            emit dataReceivedFromRobot1(fullPacket);

            // 处理缓冲区中可能存在的下一个包
            pb.buffer = pb.buffer.mid(pb.expectedSize);
            pb.receivedSize = pb.buffer.size();
            pb.expectedSize = 0;

            // 如果还有数据，递归处理
            if (pb.receivedSize > 0) {
                processROV1Buffer(QByteArray());
            } else {
                // 没有剩余数据，停止计时器
                if (pb.timeoutTimer) pb.timeoutTimer->stop();
            }
        } else {
            qDebug() << "Invalid packet footer for ROV1 | Buffer:" << pb.buffer.toHex();
            resetBuffer(ROV1);
        }
    }
}

// ROV2 (192.168.2.16) 组包处理 - 与ROV1类似但有独立缓冲区
void udpclass::processROV2Buffer(const QByteArray &data)
{
    PacketBuffer &pb = rov2Buffer;
    qDebug()<<"tes";
    // 启动/重置超时计时器
    if (pb.timeoutTimer) {
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    } else {
        pb.timeoutTimer = new QTimer(this);
        pb.timeoutTimer->setSingleShot(true);
        connect(pb.timeoutTimer, &QTimer::timeout, [this]() {
            qDebug() << "ROV2 buffer timeout, resetting";
            resetBuffer(ROV2);
        });
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    }

    if (pb.expectedSize == 0) {
        int startIndex = data.indexOf('\xFE');
        if (startIndex == -1) {
            qDebug() << "No packet header found for ROV2";
            return;
        }

        pb.buffer = data.mid(startIndex);
        pb.receivedSize = pb.buffer.size();

        if (pb.receivedSize >= 3) {
            uchar payloadLen = static_cast<uchar>(pb.buffer[2]);
            pb.expectedSize = 1 + 1 + 1 + payloadLen + 1 + 1;
            qDebug() << "New ROV2 packet started | Expected size:" << pb.expectedSize;
        }
    } else {
        pb.buffer.append(data);
        pb.receivedSize += data.size();
    }

    if (pb.expectedSize > 0 && pb.receivedSize >= pb.expectedSize) {
        QByteArray fullPacket = pb.buffer.left(pb.expectedSize);

        if (static_cast<uchar>(fullPacket[fullPacket.size() - 1]) == 0xFF) {
            qDebug() << "Complete ROV2 packet received | Size:" << fullPacket.size();
            emit dataReceivedFromRobot2(fullPacket);

            pb.buffer = pb.buffer.mid(pb.expectedSize);
            pb.receivedSize = pb.buffer.size();
            pb.expectedSize = 0;

            if (pb.receivedSize > 0) {
                processROV2Buffer(QByteArray());
            } else {
                if (pb.timeoutTimer) pb.timeoutTimer->stop();
            }
        } else {
            qDebug() << "Invalid packet footer for ROV2 | Buffer:" << pb.buffer.toHex();
            resetBuffer(ROV2);
        }
    }
}

// Buoy1 (192.168.2.19) 组包处理
void udpclass::processBuoy1Buffer(const QByteArray &data)
{
    PacketBuffer &pb = buoy1Buffer;

    if (pb.timeoutTimer) {
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    } else {
        pb.timeoutTimer = new QTimer(this);
        pb.timeoutTimer->setSingleShot(true);
        connect(pb.timeoutTimer, &QTimer::timeout, [this]() {
            qDebug() << "Buoy1 buffer timeout, resetting";
            resetBuffer(Buoy1);
        });
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    }

    if (pb.expectedSize == 0) {
        int startIndex = data.indexOf('\xFE');
        if (startIndex == -1) {
            qDebug() << "No packet header found for Buoy1";
            return;
        }

        pb.buffer = data.mid(startIndex);
        pb.receivedSize = pb.buffer.size();

        if (pb.receivedSize >= 3) {
            uchar payloadLen = static_cast<uchar>(pb.buffer[2]);
            pb.expectedSize = 1 + 1 + 1 + payloadLen + 1 + 1;
            qDebug() << "New Buoy1 packet started | Expected size:" << pb.expectedSize;
        }
    } else {
        pb.buffer.append(data);
        pb.receivedSize += data.size();
    }

    if (pb.expectedSize > 0 && pb.receivedSize >= pb.expectedSize) {
        QByteArray fullPacket = pb.buffer.left(pb.expectedSize);

        if (static_cast<uchar>(fullPacket[fullPacket.size() - 1]) == 0xFF) {
            qDebug() << "Complete Buoy1 packet received | Size:" << fullPacket.size();
            emit dataReceivedFromBuoy1(fullPacket);

            pb.buffer = pb.buffer.mid(pb.expectedSize);
            pb.receivedSize = pb.buffer.size();
            pb.expectedSize = 0;

            if (pb.receivedSize > 0) {
                processBuoy1Buffer(QByteArray());
            } else {
                if (pb.timeoutTimer) pb.timeoutTimer->stop();
            }
        } else {
            qDebug() << "Invalid packet footer for Buoy1 | Buffer:" << pb.buffer.toHex();
            resetBuffer(Buoy1);
        }
    }
}

// Buoy2 (192.168.2.25) 组包处理
void udpclass::processBuoy2Buffer(const QByteArray &data)
{
    PacketBuffer &pb = buoy2Buffer;

    if (pb.timeoutTimer) {
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    } else {
        pb.timeoutTimer = new QTimer(this);
        pb.timeoutTimer->setSingleShot(true);
        connect(pb.timeoutTimer, &QTimer::timeout, [this]() {
            qDebug() << "Buoy2 buffer timeout, resetting";
            resetBuffer(Buoy2);
        });
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    }

    if (pb.expectedSize == 0) {
        int startIndex = data.indexOf('\xFE');
        if (startIndex == -1) {
            qDebug() << "No packet header found for Buoy2";
            return;
        }

        pb.buffer = data.mid(startIndex);
        pb.receivedSize = pb.buffer.size();

        if (pb.receivedSize >= 3) {
            uchar payloadLen = static_cast<uchar>(pb.buffer[2]);
            pb.expectedSize = 1 + 1 + 1 + payloadLen + 1 + 1;
            qDebug() << "New Buoy2 packet started | Expected size:" << pb.expectedSize;
        }
    } else {
        pb.buffer.append(data);
        pb.receivedSize += data.size();
    }

    if (pb.expectedSize > 0 && pb.receivedSize >= pb.expectedSize) {
        QByteArray fullPacket = pb.buffer.left(pb.expectedSize);

        if (static_cast<uchar>(fullPacket[fullPacket.size() - 1]) == 0xFF) {
            qDebug() << "Complete Buoy2 packet received | Size:" << fullPacket.size();
            emit dataReceivedFromBuoy2(fullPacket);

            pb.buffer = pb.buffer.mid(pb.expectedSize);
            pb.receivedSize = pb.buffer.size();
            pb.expectedSize = 0;

            if (pb.receivedSize > 0) {
                processBuoy2Buffer(QByteArray());
            } else {
                if (pb.timeoutTimer) pb.timeoutTimer->stop();
            }
        } else {
            qDebug() << "Invalid packet footer for Buoy2 | Buffer:" << pb.buffer.toHex();
            resetBuffer(Buoy2);
        }
    }
}

void udpclass::processOrangepi1(const QByteArray &data)
{
    PacketBuffer &pb = Orangepi2Buffer;
    qDebug()<<"tes";
    // 启动/重置超时计时器
    if (pb.timeoutTimer) {
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    } else {
        pb.timeoutTimer = new QTimer(this);
        pb.timeoutTimer->setSingleShot(true);
        connect(pb.timeoutTimer, &QTimer::timeout, [this]() {
            qDebug() << "Orangepi2 buffer timeout, resetting";
            resetBuffer(Orangepi2);
        });
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    }

    if (pb.expectedSize == 0) {
        int startIndex = data.indexOf('\xFE');
        if (startIndex == -1) {
            qDebug() << "No packet header found for Orangepi2";
            return;
        }

        pb.buffer = data.mid(startIndex);
        pb.receivedSize = pb.buffer.size();

        if (pb.receivedSize >= 3) {
            uchar payloadLen = static_cast<uchar>(pb.buffer[2]);
            pb.expectedSize = 1 + 1 + 1 + payloadLen + 1 + 1;
            qDebug() << "New Orangepi2 packet started | Expected size:" << pb.expectedSize;
        }
    } else {
        pb.buffer.append(data);
        pb.receivedSize += data.size();
    }

    if (pb.expectedSize > 0 && pb.receivedSize >= pb.expectedSize) {
        QByteArray fullPacket = pb.buffer.left(pb.expectedSize);

        if (static_cast<uchar>(fullPacket[fullPacket.size() - 1]) == 0xFF) {
            qDebug() << "Complete Orangepi2 packet received | Size:" << fullPacket.size();
            emit dataReceivedFromOrangepi2(fullPacket);

            pb.buffer = pb.buffer.mid(pb.expectedSize);
            pb.receivedSize = pb.buffer.size();
            pb.expectedSize = 0;

            if (pb.receivedSize > 0) {
                processOrangepi2(QByteArray());
            } else {
                if (pb.timeoutTimer) pb.timeoutTimer->stop();
            }
        } else {
            qDebug() << "Invalid packet footer for Orangepi2 | Buffer:" << pb.buffer.toHex();
            resetBuffer(Orangepi2);
        }
    }
}

void udpclass::processOrangepi2(const QByteArray &data)
{
    PacketBuffer &pb = Orangepi1Buffer;
    qDebug()<<"tes";
    // 启动/重置超时计时器
    if (pb.timeoutTimer) {
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    } else {
        pb.timeoutTimer = new QTimer(this);
        pb.timeoutTimer->setSingleShot(true);
        connect(pb.timeoutTimer, &QTimer::timeout, [this]() {
            qDebug() << "Orangepi1 buffer timeout, resetting";
            resetBuffer(Orangepi1);
        });
        pb.timeoutTimer->start(BUFFER_TIMEOUT);
    }

    if (pb.expectedSize == 0) {
        int startIndex = data.indexOf('\xFE');
        if (startIndex == -1) {
            qDebug() << "No packet header found for Orangepi1";
            return;
        }

        pb.buffer = data.mid(startIndex);
        pb.receivedSize = pb.buffer.size();

        if (pb.receivedSize >= 3) {
            uchar payloadLen = static_cast<uchar>(pb.buffer[2]);
            pb.expectedSize = 1 + 1 + 1 + payloadLen + 1 + 1;
            qDebug() << "New Orangepi1 packet started | Expected size:" << pb.expectedSize;
        }
    } else {
        pb.buffer.append(data);
        pb.receivedSize += data.size();
    }

    if (pb.expectedSize > 0 && pb.receivedSize >= pb.expectedSize) {
        QByteArray fullPacket = pb.buffer.left(pb.expectedSize);

        if (static_cast<uchar>(fullPacket[fullPacket.size() - 1]) == 0xFF) {
            qDebug() << "Complete Orangepi1 packet received | Size:" << fullPacket.size();
            emit dataReceivedFromOrangepi1(fullPacket);

            pb.buffer = pb.buffer.mid(pb.expectedSize);
            pb.receivedSize = pb.buffer.size();
            pb.expectedSize = 0;

            if (pb.receivedSize > 0) {
                processOrangepi1(QByteArray());
            } else {
                if (pb.timeoutTimer) pb.timeoutTimer->stop();
            }
        } else {
            qDebug() << "Invalid packet footer for Orangepi1 | Buffer:" << pb.buffer.toHex();
            resetBuffer(Orangepi1);
        }
    }
}

// 重置指定设备的缓冲区
void udpclass::resetBuffer(DeviceType deviceType)
{
    switch (deviceType) {
    case ROV1:
        rov1Buffer.buffer.clear();
        rov1Buffer.expectedSize = 0;
        rov1Buffer.receivedSize = 0;
        if (rov1Buffer.timeoutTimer) {
            rov1Buffer.timeoutTimer->stop();
        }
        qDebug() << "ROV1 buffer reset";
        break;
    case ROV2:
        rov2Buffer.buffer.clear();
        rov2Buffer.expectedSize = 0;
        rov2Buffer.receivedSize = 0;
        if (rov2Buffer.timeoutTimer) {
            rov2Buffer.timeoutTimer->stop();
        }
        qDebug() << "ROV2 buffer reset";
        break;
    case Buoy1:
        buoy1Buffer.buffer.clear();
        buoy1Buffer.expectedSize = 0;
        buoy1Buffer.receivedSize = 0;
        if (buoy1Buffer.timeoutTimer) {
            buoy1Buffer.timeoutTimer->stop();
        }
        qDebug() << "Buoy1 buffer reset";
        break;
    case Buoy2:
        buoy2Buffer.buffer.clear();
        buoy2Buffer.expectedSize = 0;
        buoy2Buffer.receivedSize = 0;
        if (buoy2Buffer.timeoutTimer) {
            buoy2Buffer.timeoutTimer->stop();
        }
        qDebug() << "Buoy2 buffer reset";
        break;
    }
}

// 重置所有超时的缓冲区
void udpclass::resetTimedOutBuffers()
{
    auto checkAndReset = [this](PacketBuffer &buffer, DeviceType type) {
        if (buffer.expectedSize > 0 && buffer.timeoutTimer &&
            !buffer.timeoutTimer->isActive()) {
            qDebug() << "Device buffer timeout detected, resetting";
            resetBuffer(type);
        }
    };

    checkAndReset(rov1Buffer, ROV1);
    checkAndReset(rov2Buffer, ROV2);
    checkAndReset(buoy1Buffer, Buoy1);
    checkAndReset(buoy2Buffer, Buoy2);
}
