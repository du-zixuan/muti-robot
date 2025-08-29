#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H
#define MAVLINK_USE_OLD_PROTOCOL 1
#include <QObject>
#include <QKeyEvent>
#include <QDebug>
class VirtualKeyboard : public QObject
{
    Q_OBJECT
public:
    explicit VirtualKeyboard(QObject *parent = nullptr);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

signals:
    void moveForward1();   // 前进信号
    void moveBackward1();  // 后退信号
    void turnLeft1();      // 左转信号
    void turnRight1();     // 右转信号
    void Upsignal1();       // 上升信号
    void Downsignal1();       // 下降信号
    void stopForward1();   // 停止前进信号
    void stopyawForward1();// 停止航偏角
    void stopupForward1();// 停止上升下降
    void robot1_unlocak();// 机器人1电机解锁
    void robot1_locak();// 机器人1电机上锁

    void moveForward2();   // 前进信号
    void moveBackward2();  // 后退信号
    void turnLeft2();      // 左转信号
    void turnRight2();     // 右转信号
    void Upsignal2();       // 上升信号
    void Downsignal2();       // 下降信号
    void stopForward2();   // 停止前进信号
    void stopyawForward2();// 停止航偏角
    void stopupForward2();// 停止上升下降

private:
    bool isWPressed = false;  // W键是否按下
    bool isSPressed = false;  // S键是否按下
    bool isAPressed = false;  // A键是否按下
    bool isDPressed = false;  // D键是否按下
    bool isQPressed = false;  // Q键是否按下
    bool isEPressed = false;  // E键是否按下
    bool isRPressed = false;  // R键是否按下
    bool isFPressed = false;  // F键是否按下


    bool isIPressed = false;  // I键是否按下
    bool isKPressed = false;  // K键是否按下
    bool isJPressed = false;  // J键是否按下
    bool isLPressed = false;  // L键是否按下
    bool isOPressed = false;  // O键是否按下
    bool isUPressed = false;  // U键是否按下
    bool isYPressed = false;  // Y键是否按下
    bool isHPressed = false;  // H键是否按下

};

#endif // VIRTUALKEYBOARD_H
