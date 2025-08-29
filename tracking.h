#ifndef TRACKING_H
#define TRACKING_H

#include <QObject>
#include <QImage>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QDebug>
#include <QPainter>
#include <QtMath>

class Tracking : public QObject
{
    Q_OBJECT

public:
    explicit Tracking(QObject *parent = nullptr);
    ~Tracking();

    // PID参数设置
    void setPidParams(double kp, double ki, double kd);
    void setSpeedLimits(float maxYawSpeed, float maxMoveSpeed);

public slots:
    void startTracking();
    void stopTracking();
    void updateImage(const QImage &image);
    void updateDepth(float depth);
    void updateTargetPosition(quint16 x, quint16 y);  // 新增

signals:
    void controlCommand(float yawSpeed, float moveSpeed);
    void visualFeedback(QImage processedImage);

private slots:
    void pidUpdate();  // 改为私有槽


private:
    void processFrame();
    // findTargetCenter(const QImage &image);
    void drawGuidelines(QImage &image);

    QTimer *m_timer;
    bool m_trackingActive;
    QMutex m_mutex;

    // 图像和深度数据
    QImage m_currentImage;
    float m_currentDepth;
    bool m_targetDetected;
    QPoint m_targetCenter;

    // PID控制参数
    double m_kp;    // 比例系数
    double m_ki;    // 积分系数
    double m_kd;    // 微分系数
    double m_integral; // 积分项
    double m_previousError; // 上一次的偏差
    // 深度控制PID参数（新增）
    double m_depth_kp;
    double m_depth_ki;
    double m_depth_kd;
    double m_depth_integral;
    double m_depth_previousError;

    // 控制参数
    float m_maxYawSpeed;
    float m_maxMoveSpeed;
    const float TARGET_DEPTH = 0.4f; // 目标距离
};

#endif // TRACKING_H
