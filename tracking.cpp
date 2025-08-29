#include "tracking.h"

Tracking::Tracking(QObject *parent) : QObject(parent),
    m_trackingActive(false),
    m_currentDepth(0.0f),
    m_targetDetected(false),
    m_kp(0.01),   // 默认Yaw PID参数
    m_ki(0.01),
    m_kd(0.005),
    m_integral(0.0),
    m_previousError(0.0),
    m_depth_kp(0.1),  // 默认深度PID参数
    m_depth_ki(0.02),
    m_depth_kd(0.1),
    m_depth_integral(0.0),
    m_depth_previousError(0.0),
    m_maxYawSpeed(300.0f), // 默认最大角速度
    m_maxMoveSpeed(500.0f) // 默认最大移动速度
{
    m_timer = new QTimer(this);
    // 连接定时器信号到槽函数
    connect(m_timer, &QTimer::timeout, this, &Tracking::pidUpdate);
}

Tracking::~Tracking()
{
    stopTracking();
    delete m_timer;
}

void Tracking::setPidParams(double kp, double ki, double kd)
{
    QMutexLocker locker(&m_mutex);
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
}

void Tracking::setSpeedLimits(float maxYawSpeed, float maxMoveSpeed)
{
    QMutexLocker locker(&m_mutex);
    m_maxYawSpeed = maxYawSpeed;
    m_maxMoveSpeed = maxMoveSpeed;
}

void Tracking::startTracking()
{
    // 确保在正确的线程中执行
    if (QThread::currentThread() != this->thread()) {
        qWarning() << "startTracking called from wrong thread";
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (m_trackingActive) return;

    m_trackingActive = true;
    m_integral = 0.0;
    m_previousError = 0.0;

    // 启动定时器
    m_timer->start(10);
    qDebug() << "Tracking started on thread: " << QThread::currentThreadId();
}

void Tracking::stopTracking()
{
    // 确保在正确的线程中执行
    if (QThread::currentThread() != this->thread()) {
        qWarning() << "stopTracking called from wrong thread";
        return;
    }

    QMutexLocker locker(&m_mutex);
    if (!m_trackingActive) return;

    m_trackingActive = false;
    m_timer->stop();
    qDebug() << "Tracking stopped";
}

void Tracking::updateImage(const QImage &image)
{
    // 添加线程检查
    if (QThread::currentThread() != this->thread()) {
        qWarning() << "updateImage called from wrong thread";
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_currentImage = image.copy();
}

void Tracking::updateDepth(float depth)
{
    // 添加线程检查
    if (QThread::currentThread() != this->thread()) {
        qWarning() << "updateDepth called from wrong thread";
        return;
    }

    QMutexLocker locker(&m_mutex);
    m_currentDepth = depth;
}
/*QPoint Tracking::findTargetCenter(const QImage &image)
{
    // 在实际应用中应替换为真正的目标检测算法
    // 这里假设图像中有一个红色矩形框作为目标标识

    int redCount = 0;
    int centerXSum = 0;
    int centerYSum = 0;
    int minX = image.width(), maxX = 0;
    int minY = image.height(), maxY = 0;

    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            QRgb rgb = image.pixel(x, y);
            int r = qRed(rgb);
            int g = qGreen(rgb);
            int b = qBlue(rgb);

            // 简单的红色检测（实际应用中应使用真正的目标检测算法）
            if (r > 200 && g < 50 && b < 50) {
                redCount++;
                if (x < minX) minX = x;
                if (x > maxX) maxX = x;
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }

    if (redCount > 100) { // 检测到足够多的红色像素
        return QPoint((minX + maxX) / 2, (minY + maxY) / 2);
    }

    return QPoint(-1, -1); // 未检测到目标
}*/
void Tracking::updateTargetPosition(quint16 x, quint16 y)
{
    // 确保在正确的线程中执行
    if (QThread::currentThread() != this->thread()) {
        qWarning() << "updateTargetPosition called from wrong thread";
        return;
    }

    QMutexLocker locker(&m_mutex);
    // 更新目标中心点坐标
    m_targetCenter = QPoint(x, y);
    m_targetDetected = true;
}
void Tracking::drawGuidelines(QImage &image)
{
    QPainter painter(&image);
    painter.setPen(QPen(Qt::green, 2));

    // 绘制中心线
    int centerX = image.width() / 2;
    painter.drawLine(centerX, 0, centerX, image.height());

    if (m_targetDetected) {
        // 绘制目标中心点
        painter.setPen(QPen(Qt::red, 4));
        painter.drawPoint(m_targetCenter);

        // 绘制从目标到中心线的连线
        painter.setPen(QPen(Qt::yellow, 1));
        painter.drawLine(m_targetCenter, QPoint(centerX, m_targetCenter.y()));

        // 显示偏差信息
        painter.setPen(QPen(Qt::white, 2));
        int error = m_targetCenter.x() - centerX;
        painter.drawText(10, 20, QString("偏差: %1 像素").arg(error));
    }
}

void Tracking::processFrame()
{
    if (m_currentImage.isNull())
    {
        qDebug()<<"The img is null";
        return;
    }

    // 查找目标中心点
    //m_targetCenter = findTargetCenter(m_currentImage);
    //m_targetDetected = (m_targetCenter.x() >= 0);

    // 添加视觉反馈
    QImage visualImage = m_currentImage.copy();
    drawGuidelines(visualImage);
    emit visualFeedback(visualImage);
}

void Tracking::pidUpdate()
{
    qDebug()<<"开始PID";
    QMutexLocker locker(&m_mutex);
    if (!m_trackingActive) return;

    // 处理当前帧
    processFrame();
    qDebug()<<"开始PID2";
    float yawSpeed = 0.0f;
    float moveSpeed = 0.0f;

    // Yaw控制（仅当检测到目标时）
    if (m_targetDetected) {
        int imageCenterX = m_currentImage.width() / 2;
        double error = m_targetCenter.x() - imageCenterX;

        // PID计算
        m_integral += error;
        double derivative = error - m_previousError;
        double output = m_kp * error + m_ki * m_integral + m_kd * derivative;
        m_previousError = error;

        // 限制输出范围
        yawSpeed = qBound(-m_maxYawSpeed, static_cast<float>(output), m_maxYawSpeed);

        qDebug() << "PID输出 - 误差:" << error
                 << " 积分:" << m_integral
                 << " 微分:" << derivative
                 << " 输出:" << output;
    }

    // 深度控制（前后方向）- 使用PID替代开关控制
    if (m_currentDepth > 0) {
        double depthError = m_currentDepth - TARGET_DEPTH;

        // PID计算
        m_depth_integral += depthError;
        double depthDerivative = depthError - m_depth_previousError;
        double depthOutput = m_depth_kp * depthError
                             + m_depth_ki * m_depth_integral
                             + m_depth_kd * depthDerivative;
        m_depth_previousError = depthError;

        moveSpeed = qBound(-m_maxMoveSpeed, static_cast<float>(depthOutput), m_maxMoveSpeed);

        qDebug() << "Depth PID - 误差:" << depthError
                 << " 输出:" << depthOutput;
    }
    // 发送控制命令
    emit controlCommand(yawSpeed, moveSpeed);
}
