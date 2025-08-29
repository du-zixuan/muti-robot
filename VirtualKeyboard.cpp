#include "VirtualKeyboard.h"

VirtualKeyboard::VirtualKeyboard(QObject *parent)
    : QObject{parent}
{

}
bool VirtualKeyboard::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_W && !isWPressed) {
            isWPressed = true;
            emit moveForward1();
            qDebug() << "Move Forward";
        }
        else if (keyEvent->key() == Qt::Key_S && !isSPressed) {
            isSPressed = true;
            emit moveBackward1();
            qDebug() << "Move Backward";
        }
        else if (keyEvent->key() == Qt::Key_A && !isAPressed) {
            isAPressed = true;
            emit turnLeft1();
            qDebug() << "Turn Left";
        }
        else if (keyEvent->key() == Qt::Key_D && !isDPressed) {
            isDPressed = true;
            emit turnRight1();
            qDebug() << "Turn Right";
        }
        else if (keyEvent->key() == Qt::Key_Q && !isQPressed) {
            isQPressed = true;
            emit Upsignal1();
            qDebug() << "Turn Up";
        }
        else if (keyEvent->key() == Qt::Key_E && !isEPressed) {
            isEPressed = true;
            emit Downsignal1();
            qDebug() << "Turn Down";
        }
        else if (keyEvent->key() == Qt::Key_R && !isRPressed) {
            isRPressed = true;
            emit robot1_unlocak();
            qDebug() << "Turn unlocak";
        }
        else if (keyEvent->key() == Qt::Key_R && isRPressed) {
            isRPressed = false;
            emit robot1_locak();
            qDebug() << "Turn locak";
        }




        if (keyEvent->key() == Qt::Key_I && !isIPressed) {
            isIPressed = true;
            emit moveForward2();
            qDebug() << "robot2 Move Forward";
        }
        else if (keyEvent->key() == Qt::Key_K && !isKPressed) {
            isKPressed = true;
            emit moveBackward2();
            qDebug() << "robot2 Move Backward";
        }
        else if (keyEvent->key() == Qt::Key_J && !isJPressed) {
            isJPressed = true;
            emit turnLeft2();
            qDebug() << "robot2 Turn Left";
        }
        else if (keyEvent->key() == Qt::Key_L && !isLPressed) {
            isLPressed = true;
            emit turnRight2();
            qDebug() << "robot2 Turn Right";
        }
        else if (keyEvent->key() == Qt::Key_O && !isOPressed) {
            isOPressed = true;
            emit Downsignal2();
            qDebug() << "robot2 Turn Down";
        }
        else if (keyEvent->key() == Qt::Key_U && !isUPressed) {
            isUPressed = true;
            emit Upsignal2();
            qDebug() << "robot2 Turn Up";
        }
    }
    else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_W) {
            isWPressed = false;
            emit stopForward1();
        }
        else if (keyEvent->key() == Qt::Key_S) {
            isSPressed = false;
            emit stopForward1();
        }
        else if (keyEvent->key() == Qt::Key_A) {
            isAPressed = false;
            emit stopyawForward1();
        }
        else if (keyEvent->key() == Qt::Key_D) {
            isDPressed = false;
            emit stopyawForward1();
        }
        else if (keyEvent->key() == Qt::Key_Q) {
            isQPressed = false;
            emit stopupForward1();
        }
        else if (keyEvent->key() == Qt::Key_E) {
            isEPressed = false;
            emit stopupForward1();
        }




        if (keyEvent->key() == Qt::Key_I) {
            isIPressed = false;
            emit stopForward2();
        }
        else if (keyEvent->key() == Qt::Key_K) {
            isKPressed = false;
            emit stopForward2();
        }
        else if (keyEvent->key() == Qt::Key_J) {
            isJPressed = false;
            emit stopyawForward2();
        }
        else if (keyEvent->key() == Qt::Key_L) {
            isLPressed = false;
            emit stopyawForward2();
        }
        else if (keyEvent->key() == Qt::Key_O) {
            isOPressed = false;
            emit stopupForward2();
        }
        else if (keyEvent->key() == Qt::Key_U) {
            isUPressed = false;
            emit stopupForward2();
        }
    }
    return QObject::eventFilter(watched, event); // 让其他事件继续传播
}
