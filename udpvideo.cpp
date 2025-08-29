#include "udpvideo.h"
#include <QDebug>
#include <QDateTime>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/utility.hpp>


udpvideo::udpvideo(QObject *parent, quint16 port)
    : QObject{parent}, listenPort(port)
{
    udpSocket = new QUdpSocket(this);
    udpSocket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, 20 * 1024 * 1024);

    // 绑定到特定 IP 地址 192.168.2.13，指定的端口
    QHostAddress bindAddress("192.168.2.13");

    if (!udpSocket->bind(bindAddress, listenPort, QUdpSocket::ShareAddress)) {
        qDebug() << "Failed to bind UDP socket to" << bindAddress.toString() << ":" << listenPort
                 << "| Error:" << udpSocket->errorString();
    } else {
        qDebug() << "UDP video socket bound to" << bindAddress.toString() << ":" << listenPort;
        connect(udpSocket, &QUdpSocket::readyRead, this, &udpvideo::readPendingDatagrams);
    }

    // 设置缓冲区超时计时器
    timeoutTimer = new QTimer(this);
    connect(timeoutTimer, &QTimer::timeout, this, &udpvideo::resetTimedOutBuffers);
    timeoutTimer->start(1000); // 每秒检查一次
}

udpvideo::~udpvideo()
{
    if (timeoutTimer) {
        timeoutTimer->stop();
    }
    udpSocket->close();
}

void udpvideo::startReceiving()
{
    if (!udpSocket->isValid() && !udpSocket->bind(QHostAddress("192.168.2.13"), listenPort, QUdpSocket::ShareAddress)) {
        qDebug() << "Failed to start video receiving on port" << listenPort;
    }
}
void udpvideo::readPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        qint64 bytesRead = udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (bytesRead < 6) continue; // 数据包太小，忽略

        QString senderIp = sender.toString();

        // 解析数据包头 (4字节帧ID + 2字节包信息)
        if (datagram.size() < 6) continue;

        quint32 frameId = *reinterpret_cast<const quint32*>(datagram.constData());
        quint16 packetInfo = *reinterpret_cast<const quint16*>(datagram.constData() + 4);

        QMutexLocker locker(&bufferMutex);

        // 确保发送者缓冲区存在
        if (!frameBuffers.contains(senderIp)) {
            frameBuffers[senderIp] = FrameBuffer();
            frameBuffers[senderIp].timer.start();
        }

        FrameBuffer &frameBuffer = frameBuffers[senderIp];

        // 帧头包 (包含总包数)你的这个基本三角关系里面写错了吧，你给的三角形和图片不匹配，还有你的focalLength和sensorWidth是什么长度，这两个有什么用还是没说清楚
        if (datagram.size() == 6 && packetInfo != 0xFFFF) {
            frameBuffer.frameId = frameId;
            frameBuffer.totalPackets = packetInfo;
            frameBuffer.packets.clear();
            frameBuffer.timer.start();
            frameBuffer.frameEndReceived = false;
            frameBuffer.frameHeaderReceived = true;
            qDebug() << "Frame header received from" << senderIp << "frameId:" << frameId
                     << "total packets:" << packetInfo;
            continue;
        }

        // 帧尾包 - 包含浮漂反馈数据和深度信息
        // 在 readPendingDatagrams 函数中修改帧尾包解析
        if (datagram.size() >= 30 && packetInfo == 0xFFFF) { // 最小长度: 6 + 16 + 4 + 4 = 30字节
            if (frameBuffer.frameId == frameId && frameBuffer.frameHeaderReceived) {
                frameBuffer.frameEndReceived = true;

                // 提取浮漂反馈数据 (16字节)
                frameBuffer.floatFeedback = datagram.mid(6, 16);

                // 提取深度值 (4字节)
                if (datagram.size() >= 30) {
                    // 从22字节位置读取4字节深度值
                    QByteArray depthBytes = datagram.mid(22, 4);

                    // 确保字节序正确转换
                    if (QSysInfo::ByteOrder == QSysInfo::LittleEndian) {
                        std::reverse(depthBytes.begin(), depthBytes.end());
                    }
                    memcpy(&frameBuffer.depthValue, depthBytes.constData(), sizeof(float));

                    // 从26字节位置读取中心点坐标 (2字节x + 2字节y)
                    // 注意：网络字节序是大端，我们需要转换成本机字节序
                    frameBuffer.targetX = qFromBigEndian<quint16>(
                        reinterpret_cast<const uchar*>(datagram.constData() + 26));
                    frameBuffer.targetY = qFromBigEndian<quint16>(
                        reinterpret_cast<const uchar*>(datagram.constData() + 28));
                }

                qDebug() << "Frame end received with target position: ("
                         << frameBuffer.targetX << "," << frameBuffer.targetY << ")";

                locker.unlock();
                processReceivedFrame(senderIp, frameId);
                locker.relock();
            }
            continue;
        }

        // 数据包 (长度大于6)
        if (datagram.size() > 6) {
            // 确保帧缓冲区存在且ID匹配
            if (!frameBuffer.frameHeaderReceived || frameBuffer.frameId != frameId) {
                continue;
            }

            // 获取包索引
            quint16 packetIdx = packetInfo;

            // 只存储新包，避免重复处理
            if (!frameBuffer.packets.contains(packetIdx)) {
                // 存储数据包内容（去掉6字节包头）
                frameBuffer.packets[packetIdx] = datagram.mid(6);
                //qDebug() << "Packet" << packetIdx << "received for frameId:" << frameId
                 //        << "from" << senderIp << "size:" << frameBuffer.packets[packetIdx].size();

                // 如果已知总包数且收到所有包，尝试重建
                if (frameBuffer.totalPackets > 0 &&
                    frameBuffer.packets.size() == frameBuffer.totalPackets) {
                    locker.unlock();
                    processReceivedFrame(senderIp, frameId);
                    locker.relock();
                }
            }
        }
    }
}

void udpvideo::processReceivedFrame(const QString &senderIp, quint32 frameId)
{
    QMutexLocker locker(&bufferMutex);

    if (!frameBuffers.contains(senderIp) || frameBuffers[senderIp].frameId != frameId) {
        return;
    }

    FrameBuffer &frameBuffer = frameBuffers[senderIp];

    // 检查是否可显示 (收到帧尾或收齐所有包)
    bool is_complete = frameBuffer.frameEndReceived;
    bool all_packets_received = frameBuffer.totalPackets > 0 &&
                                frameBuffer.packets.size() == frameBuffer.totalPackets;

    if (!(is_complete || all_packets_received)) {
        qDebug() << "Frame not complete, cannot reconstruct";
        return;
    }

    // 重组帧数据 - 按包索引排序
    QByteArray fullData;
    QList<quint16> keys = frameBuffer.packets.keys();
    std::sort(keys.begin(), keys.end());

    for (quint16 key : keys) {
        fullData.append(frameBuffer.packets[key]);
    }

    // 如果帧尾包已收到但总包数未知，可能需要裁剪多余数据
    if (frameBuffer.totalPackets == 0 && frameBuffer.frameEndReceived) {
        // 查找JPEG结束标记
        int endPos = fullData.indexOf("\xFF\xD9");
        if (endPos != -1) {
            fullData = fullData.left(endPos + 2);
        }
    }

    // 处理浮漂反馈数据
    if (!frameBuffer.floatFeedback.isEmpty() && frameBuffer.floatFeedback.size() >= 16) {
        const char *data = frameBuffer.floatFeedback.constData();
        // 解析4个float (每个4字节)
        float f1 = *reinterpret_cast<const float*>(data);
        float f2 = *reinterpret_cast<const float*>(data + 4);
        float f3 = *reinterpret_cast<const float*>(data + 8);
        float f4 = *reinterpret_cast<const float*>(data + 12);

        QVector<float> feedback = {f1, f2, f3, f4};
        floatFeedbackMap[senderIp] = feedback;
        emit floatFeedbackReceived(feedback, senderIp);
        qDebug() << "Float feedback received from" << senderIp << ":" << f1 << f2 << f3 << f4;
    }

    // 保存深度信息到局部变量
    float currentDepth = frameBuffer.depthValue;
    depthInfoMap[senderIp] = currentDepth;
    // 保存目标位置
    quint16 currentTargetX = frameBuffer.targetX;
    quint16 currentTargetY = frameBuffer.targetY;
    // 移除缓冲区
    frameBuffers.remove(senderIp);
    locker.unlock();

    // 解码图像
    QImage fullImage = QImage::fromData(fullData, "JPEG");
    if (fullImage.isNull()) {
        qWarning() << "Failed to decode image from" << senderIp << "frameId:" << frameId;
        return;
    }

    qDebug() << "Image reconstructed from" << senderIp << "frameId:" << frameId
             << "size:" << fullImage.size() << "data size:" << fullData.size();

    // 发送图像给界面显示
    emit leftImageReceived(fullImage, senderIp);

    // 发送深度信息 - 使用局部变量
    qDebug() << "深度信息为：" << currentDepth;
    emit depthInfoReceived(currentDepth, senderIp);
    // 发送目标位置信息
    emit targetPositionReceived(currentTargetX, currentTargetY, senderIp);
}

void udpvideo::resetTimedOutBuffers()
{
    QMutexLocker locker(&bufferMutex);
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    // 清理超时帧 (超过500ms)
    QMutableHashIterator<QString, FrameBuffer> i(frameBuffers);
    while (i.hasNext()) {
        i.next();
        FrameBuffer &frameBuffer = i.value();

        if (frameBuffer.timer.hasExpired(500)) {
            qDebug() << "Removing timed out frame" << frameBuffer.frameId
                     << "from" << i.key()
                     << "received packets:" << frameBuffer.packets.size()
                     << "expected:" << frameBuffer.totalPackets;
            i.remove();
        }
    }
}
