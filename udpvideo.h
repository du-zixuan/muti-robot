#ifndef UDPVIDEO_H
#define UDPVIDEO_H

#include <QObject>
#include <QUdpSocket>
#include <QImage>
#include <QHash>
#include <QByteArray>
#include <QTimer>
#include <QThread>
#include <QMutex>
#include <opencv2/opencv.hpp>
#include <QElapsedTimer>
#include <QVector>
#include <QMap>
#include <QtEndian>

class udpvideo : public QObject
{
    Q_OBJECT
public:
    explicit udpvideo(QObject *parent = nullptr, quint16 port = 8888);
    ~udpvideo();
    void startReceiving();

signals:
    void leftImageReceived(QImage image, const QString &senderIp);
    void depthInfoReceived(float depth, const QString &senderIp);
    void floatFeedbackReceived(const QVector<float> &feedback, const QString &senderIp);
    void targetPositionReceived(quint16 x, quint16 y, const QString &senderIp);
    
private slots:
    void readPendingDatagrams();
    void resetTimedOutBuffers();

private:
    QUdpSocket *udpSocket = nullptr;
    QTimer *timeoutTimer = nullptr;
    quint16 listenPort;

    // 帧缓冲区结构
    struct FrameBuffer {
        quint32 frameId;
        quint16 totalPackets = 0;
        QMap<quint16, QByteArray> packets;
        QElapsedTimer timer;
        bool frameEndReceived = false;
        bool frameHeaderReceived = false;
        QByteArray floatFeedback;
        float depthValue = 0.0f;
        quint16 targetX = 0;
        quint16 targetY = 0;
    };

    // 每个发送者IP的帧缓冲区
    QHash<QString, FrameBuffer> frameBuffers;
    QMutex bufferMutex;

    // 全局变量存储深度和浮漂信息
    QHash<QString, float> depthInfoMap;
    QHash<QString, QVector<float>> floatFeedbackMap;

    void processReceivedFrame(const QString &senderIp, quint32 frameId);
};

#endif // UDPVIDEO_H
