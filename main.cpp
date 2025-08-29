#include "widget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //加载样式表
    QFile file(":/css/qss/NeonButtons.qss");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        qApp->setStyleSheet(qss);
        file.close();
    }

    //全局字体
    QFont font("微软雅黑", 10);
    a.setFont(font);
    Widget w;
    w.show();
    return a.exec();
}
