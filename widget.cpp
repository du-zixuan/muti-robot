namespace Ui {
class Widget;  // 添加UI类的前置声明
}
#include "widget.h"
#include "ui_widget.h"
#include <QGamepad>
#include <QGamepadManager>
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setFixedSize(1850, 1010);
    // 设置窗口获取焦点，确保能够接收到键盘事件
    this->setFocusPolicy(Qt::StrongFocus);  // 重要，确保能接收到键盘事件
    //加载图片
    //ui->hit_jpg->setPixmap(QPixmap("/home/yxr998/haishenQT/th-removebg-preview.png").scaled(ui->hit_jpg->width(), ui->hit_jpg->height()));
    // 在Widget::Widget构造函数中，ui->setupUi(this);之后添加：
    // 设置全局样式表
    this->setStyleSheet(R"(
/* 主窗口样式 - 工业风格深色背景 */
QWidget {
    background-color: #2c3e50;
    color: #ecf0f1;
    font-family: 'Segoe UI', Arial, sans-serif;
    font-size: 10pt;
}

/* 分组框样式 - 金属质感 */
QGroupBox {
    border: 2px solid #7f8c8d;
    border-radius: 6px;
    margin-top: 1ex;
    font-size: 11pt;
    font-weight: bold;
    color: #ecf0f1;
    background-color: #34495e;
}

QGroupBox::title {
    subcontrol-origin: margin;
    subcontrol-position: top center;
    padding: 4px 10px;
    background-color: #3498db;
    color: white;
    border-radius: 4px;
    font-weight: bold;
}
PacketBuffer_ROV
/* 按钮样式 - 工业蓝 */
QPushButton {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);processBuoy1Buffer
    border: 1px solid #2980b9;
    border-radius: 4px;
    color: white;
    padding: 6px 12px;
    font-weight: bold;
    font-size: 10pt;
    min-width: 80px;
}

QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3ca0db, stop:1 #2c8bc9);
    border: 1px solid #3498db;
}

QPushButton:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #2980b9, stop:1 #2573a7);
}

/* 输入框样式 - 工业深色 */
QLineEdit {
    background-color: #2c3e50;
    border: 1px solid #7f8c8d;
    border-radius: 4px;
    padding: 5px;
    color: #ecf0f1;
    font-size: 11pt;
    selection-background-color: #3498db;
}processBuoy1Buffer

/* 标签样式 - 高对比度白色 */
QLabel {
    color: #ecf0f1;
    font-size: 10pt;
    font-weight: bold;
}

/* 单选按钮样式 */
QRadioButton {
    color: #ecf0f1;
    font-size: 10pt;
}

QRadioButton::indicator {
    width: 16px;
    height: 16px;
    border-radius: 8px;
    border: 2px solid #3498db;
    background: #2c3e50;
}processBuoy1Buffer

QRadioButton::indicator:checked {
    background-color: #3498db;
}

/* 选项卡样式 - 工业风格 */
QTabWidget::pane {
    border: 1px solid #7f8c8d;
    background: #34495e;
    border-radius: 6px;
}

QTabBar::tab {
    background: #2c3e50;
    border: 1px solid #7f8c8d;
    border-bottom: none;
    border-top-left-radius: 6px;
    border-top-right-radius: 6px;
    padding: 6px 15px;
    color: #bdc3c7;
    font-weight: bold;
}

QTabBar::tab:selected {
    background: #34495e;
    color: #ecf0f1;
    border-color: #7f8c8d;
}

/* 图表视图样式 */
QGraphicsView {
    background: #2c3e50;
    border: 1px solid #7f8c8d;
    border-radius: 6px;
}

/* 堆叠窗口样式 */
QStackedWidget {
    background: transparent;
    border: none;
}

/* 进度条样式 */
QProgressBar {
    border: 1px solid #7f8c8d;
    border-radius: 5px;
    background: #2c3e50;
    text-align: center;
    color: white;
    font-weight: bold;
}

QProgressBar::chunk {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #3498db, stop:1 #2980b9);
    border-radius: 4px;
}

/* 状态指示器样式 */
QLineEdit#robot1_connect_state,
QLineEdit#robot2_connect_state {
    border-radius: 10px;
    min-height: 25px;
    max-height: 25px;
    text-align: center;
    font-weight: bold;
    border: 1px solid #7f8c8d;
}
)");
    //################################################
    //#                                              #
    //#           界面初始化                          #
    //#                                             #
    //#                                              #
    //################################################
    Widget_init();
    // 创建跟踪模块和线程 - 必须在所有信号连接之前

    //创建udp线程和udp实例化对象
    u1 = new QThread();
    udp =new udpclass();
    tr1= new QThread();
    pathPlanner= new traversal();
    v1 = new QThread();
    v2 = new QThread(); // 添加第二个视频线程
    udp_video1 = new udpvideo(nullptr, 8888); // 原始ROV
    udp_video2 = new udpvideo(nullptr, 8887); // 新ROV 192.168.2.18
    udp->moveToThread(u1);
    u1->start();
    pathPlanner->moveToThread(tr1);
    tr1->start();
    udp_video1->moveToThread(v1);
    v1->start();
    udp_video2->moveToThread(v2); // 启动第二个视频线程
    v2->start();

    // 连接UDP接收信号到不同的处理函数
    connect(udp, &udpclass::dataReceivedFromRobot1, this, &Widget::handleUdpData1);
    connect(udp, &udpclass::dataReceivedFromRobot2, this, &Widget::handleUdpData2);
    connect(udp,&udpclass::dataReceivedFromBuoy1,this,&Widget::handleBuoy1Data);
    connect(udp,&udpclass::dataReceivedFromBuoy2,this,&Widget::handleBuoy2Data);
    // 在构造函数中添加：
    // 连接视频信号
    connect(udp_video1, &udpvideo::leftImageReceived, this, &Widget::updateCameraView);
    connect(udp_video1, &udpvideo::depthInfoReceived, this, &Widget::updateDepthInfo);
    // 连接目标位置信号
    connect(udp_video1, &udpvideo::targetPositionReceived,
            this, &Widget::updateTargetPosition);
    
    connect(udp_video2, &udpvideo::leftImageReceived, this, &Widget::updateCameraView);
    connect(udp_video2, &udpvideo::depthInfoReceived, this, &Widget::updateDepthInfo);
            
    
    // 在构造函数中添加连接
    //connect(udp_video, &udpvideo::floatFeedbackReceived, this, &Widget::handleFloatFeedback);
    connect(udp, &udpclass::dataReceivedFromOther, this, &Widget::handleOtherUdpData);
    //################################################
    //#                                              #
    //#           手柄事件  #
    //#
    //#                                              #
    //################################################
    //手柄初始化
    auto gamepads=QGamepadManager::instance()->connectedGamepads();
    m_gamepad=new QGamepad(*gamepads.begin(),this);
    //x和y平动控制
    connect(m_gamepad,&QGamepad::axisLeftYChanged,this,&Widget::shoubing_pingdong_x);
    connect(m_gamepad,&QGamepad::axisLeftXChanged,this,&Widget::shoubing_pingdong_y);
    //z平动控制即上升下潜
    connect(m_gamepad,&QGamepad::axisRightYChanged,this,&Widget::shoubing_pingdong_z);
    //俯仰（pitch）和左右（yaw）侧翻(roll)控制
    connect(m_gamepad,&QGamepad::buttonXChanged,this,&Widget::shoubing_bulletmove);
    connect(m_gamepad,&QGamepad::buttonBChanged,this,&Widget::shoubing_roll_open);
    connect(m_gamepad,&QGamepad::axisRightXChanged,this,&Widget::shoubing_zhuandong_yaw);
    connect(m_gamepad,&QGamepad::buttonAChanged,this,&Widget::shoubing_free_move);
    connect(m_gamepad,&QGamepad::buttonYChanged,this,&Widget::shoubing_dingshen);
    //电机上锁
    connect(m_gamepad,&QGamepad::buttonGuideChanged,this,&Widget::robot1_locak);

    // 设置PID参数（根据实际调试调整）
    //m_tracking->setPidParams(0.15, 0.02, 0.08);
    //m_tracking->setSpeedLimits(400.0f, 600.0f);
    //################################################
    //#                                              #
    //#           伺服跟踪事件            #
    //#
    //#                                              #
    //################################################
    // 创建跟踪模块和线程
    m_trackingThread = new QThread(this);
    m_tracking = new Tracking();

    // 确保所有操作都在目标线程中执行
    connect(m_trackingThread, &QThread::started, [this]() {
        qDebug() << "Tracking thread started";
    });

    // 将跟踪对象移动到新线程
    m_tracking->moveToThread(m_trackingThread);

    // 启动线程
    m_trackingThread->start();

    // 设置PID参数
    QMetaObject::invokeMethod(m_tracking, [this]() {
        m_tracking->setPidParams(0.15, 0.02, 0.08);
        m_tracking->setSpeedLimits(400.0f, 600.0f);
    }, Qt::QueuedConnection);

    // 连接信号
    connect(m_tracking, &Tracking::controlCommand, this, &Widget::handleTrackingCommand);
    connect(m_tracking, &Tracking::visualFeedback, this, &Widget::updateTrackingVisual);

    // 连接跟踪按钮
    connect(ui->Pushutton_track, &QPushButton::clicked, this, &Widget::on_pushButton_track_clicked);

    // 视频和深度信号转发
    connect(udp_video1, &udpvideo::leftImageReceived, this, [this](QImage img, const QString &senderIp) {
        if (senderIp == "192.168.2.24") {
            QMetaObject::invokeMethod(m_tracking, "updateImage", Qt::QueuedConnection,
                                      Q_ARG(QImage, img));
        }
    });
    
    connect(udp_video1, &udpvideo::depthInfoReceived, this, [this](float depth, const QString &senderIp) {
        if (senderIp == "192.168.2.24") {
            QMetaObject::invokeMethod(m_tracking, "updateDepth", Qt::QueuedConnection,
                                      Q_ARG(float, depth));
        }
    });


    //################################################
    //#                                              #
    //#           数据传输事件            #
    //#
    //#                                              #
    //################################################
    this->timer_for_move=new QTimer(this);
    timer_for_move->start(10);
    //################################################
    //#                                              #
    //#             ui控件信号与槽建立                 #
    //#                                             #
    //#                                              #
    //################################################
    init_sign_slots();
    // 确保视频接收已启动
    udp_video1->startReceiving();
    udp_video2->startReceiving(); // 启动第二个视频接收

    // 在构造函数末尾添加
    // 简化阴影效果，工业风格更注重功能而非花哨效果
    QGraphicsDropShadowEffect *controlShadow = new QGraphicsDropShadowEffect(this);
    controlShadow->setBlurRadius(8);
    controlShadow->setColor(QColor(0, 0, 0, 160));
    controlShadow->setOffset(2, 2);
    ui->groupBox_control->setGraphicsEffect(controlShadow);

    QGraphicsDropShadowEffect *mapShadow = new QGraphicsDropShadowEffect(this);
    mapShadow->setBlurRadius(8);
    mapShadow->setColor(QColor(0, 0, 0, 160));
    mapShadow->setOffset(2, 2);
    ui->groupBox_map->setGraphicsEffect(mapShadow);

    QGraphicsDropShadowEffect *chartShadow = new QGraphicsDropShadowEffect(this);
    chartShadow->setBlurRadius(8);
    chartShadow->setColor(QColor(0, 0, 0, 160));
    chartShadow->setOffset(2, 2);
    ui->graphicsView_xyz->setGraphicsEffect(chartShadow);
    //ui->graphicsView_jiaodu->setGraphicsEffect(chartShadow->clone());

}
// 供QML调用的槽函数
void Widget::addWaypoint(double lat, double lon) {
    QGeoCoordinate coord(lat, lon);
    waypoints.append(coord);
    qDebug() << "Waypoint added:" << coord;
}

//void robot1_unlocak();
//void robot1_locak();

//################################################
//#                                              #
//#      实例化控件的信号与槽函数                   #
//#                                             #
//#                                              #
//################################################
void Widget::init_sign_slots()
{
    connect(ui->radioButton_xyz_data, &QRadioButton::toggled, this, &Widget::onRadioButtonxyz);
    connect(ui->radioButton_xyz_graph, &QRadioButton::toggled, this, &Widget::onRadioButtonxyz);
    connect(ui->robot1_date, &QRadioButton::toggled, this, &Widget::onRadioButtonpage);
    connect(ui->robot2_date, &QRadioButton::toggled, this, &Widget::onRadioButtonpage);
    connect(timer_for_move,&QTimer::timeout,this,&Widget::print_move_data);
    connect(ui->robot1_connect,&QPushButton::clicked,this,&Widget::onrobot1_connect);
    connect(ui->robot2_connect,&QPushButton::clicked,this,&Widget::onrobot2_connect);
    //自主遍历模式
    connect(ui->pushButton_save,&QPushButton::clicked,this,&Widget::traversal_start);
}
void Widget::onrobot1_connect()
{
    is_connect=1;
    qDebug()<<"is_connect:"<<is_connect;
}
void Widget::onrobot2_connect()
{
    is_connect=2;
    qDebug()<<"is_connect:"<<is_connect;
}
void Widget::onRadioButtonxyz()
{
    if (ui->radioButton_xyz_graph->isChecked()) {
        // 切换到 page1
        ui->stackedWidget_xyz->setCurrentIndex(0);
        ui->stackedWidget_jiaodu->setCurrentIndex(0);
    } else if (ui->radioButton_xyz_data->isChecked()) {
        // 切换到 page2
        ui->stackedWidget_xyz->setCurrentIndex(1);
        ui->stackedWidget_jiaodu->setCurrentIndex(1);
    }
}
void Widget::onRadioButtonpage()
{
    if (ui->robot1_date->isChecked()) {
        // 切换到 page1
        ui->stackedWidget_shouye->setCurrentIndex(0);
    } else if (ui->robot2_date->isChecked()) {
        // 切换到 page2
        ui->stackedWidget_shouye->setCurrentIndex(1);
    }
}
void Widget::traversal_start()
{
    // 检查是否已获取浮漂1的GPS数据
    if (GPS_Buoy1_state[0] == 0.0 && GPS_Buoy1_state[1] == 0.0) {
        qDebug() << "No GPS data from Buoy1 yet!";
        return;
    }

    qDebug() << "Starting boustrophedon path planning for ROV1";
    qDebug() << "Start Lat:" << GPS_Buoy1_state[0] << "Lon:" << GPS_Buoy1_state[1];
    qDebug() << "Heading:" << rov1_heading << "Width:" << SEARCH_WIDTH
             << "Height:" << SEARCH_HEIGHT << "Strip width:" << STRIP_WIDTH;

    // 生成路径点
    waypoint_ROV1 = pathPlanner->generateBoustrophedonPath(
        GPS_Buoy1_state[0], GPS_Buoy1_state[1],
        rov1_heading, SEARCH_WIDTH, SEARCH_HEIGHT, STRIP_WIDTH);

    // 打印生成的路径点
    qDebug() << "Generated path with" << waypoint_ROV1.size() << "waypoints:";
    qDebug() << "初始化坐标点为：" << GPS_Buoy1_state[0]<<"and"<<GPS_Buoy1_state[1];
    for (int i = 0; i < waypoint_ROV1.size(); i++) {
        qDebug() << "Waypoint" << i << ": Lat:" << waypoint_ROV1[i].latitude()
        << "Lon:" << waypoint_ROV1[i].longitude();
    }

    // 可视化路径
    visualizePathOnMap();

    // 发送路径给ROV1
    sendPathToROV1();
}

void Widget::visualizePathOnMap()
{
    // 获取QML地图对象
    QObject *mapObject = ui->mapWidget->rootObject();

    // 清除现有路径
    QMetaObject::invokeMethod(mapObject, "clearPath");

    // 添加新路径点
    for (const QGeoCoordinate &point : waypoint_ROV1) {
        QMetaObject::invokeMethod(mapObject, "addWaypoint",
                                  Q_ARG(QVariant, point.latitude()),
                                  Q_ARG(QVariant, point.longitude()));
    }

    // 绘制路径
    QMetaObject::invokeMethod(mapObject, "drawPath");

    qDebug() << "Path visualized on map";
}
//################################################
//#                                              #
//#           手柄事件  #
//#
//#                                              #
//################################################
//-------------------------------x和y平动控制----------------------------------------------------
void Widget::shoubing_pingdong_x(double value){
    if(value < 0.1 && value > -0.1){   //认定为0
        //p1gamepad->set_lineEdit_pingdong_x(0);

        ui->robot1_x_input->setText(QString::number(0));
    }
    else if(value>=0.1){    //把0.1～1.0映射到-400～0
        double num = 0.0+1000*(value-0.1)/0.9*(-1.0);
        ui->robot1_x_input->setText(QString::number(num));
    }
    else if(value<=-0.1){   //把-1.0～-0.1映射到0～400
        double num = 0.0+1000*(value+0.1)/0.9*(-1.0);
        ui->robot1_x_input->setText(QString::number(num));
    }
}
void Widget::shoubing_pingdong_y(double value){
    if(value < 0.1 && value > -0.1){   //认定为0
        ui->robot1_y_input->setText(QString::number(0));
    }
    else if(value>=0.1){    //把0.1～1.0映射到0～400，y要反转一下
        double num = 0.0+1000*(value-0.1)/0.9*(1.0);
        ui->robot1_y_input->setText(QString::number(num));
        //qDebug()<<"value:"<<value;
    }

    else if(value<=-0.1){   //把-1.0～-0.1映射到-400～0，y要反转一下
        double num = 0.0+1000*(value+0.1)/0.9*(1.0);
        ui->robot1_y_input->setText(QString::number(num));
    }
}
//-------------------------------z平动控制即上升下潜-------------------------------------------
void Widget::shoubing_pingdong_z(double value)
{
    if(value < 0.1 && value > -0.1){   //认定为0
        ui->robot1_z_input->setText(QString::number(500));
    }
    else if(value>=0.1){    //把0.1～1.0映射到0～400，y要反转一下
        double num = 500.0+500*(value-0.1)/0.9*(-1.0);
        ui->robot1_z_input->setText(QString::number(num));
    }

    else if(value<=-0.1){   //把-1.0～-0.1映射到-400～0，y要反转一下
        double num = 500.0+500*(value+0.1)/0.9*(-1.0);
        ui->robot1_z_input->setText(QString::number(num));
    }
}

//-------------------------------俯仰（pitch）和左右（yaw）控制----------------------------------
void Widget::shoubing_zhuandong_yaw(double value){
    if(value < 0.1 && value > -0.1){   //认定为0
        ui->robot1_yaw_input->setText(QString::number(0));
    }
    else if(value>=0.1){    //把0.1～1.0映射到0～400
        double num = 0.0+1000*(value-0.1)/0.9;
        ui->robot1_yaw_input->setText(QString::number(num));
    }
    else if(value<=-0.1){   //把-1.0～-0.1映射到-400～0
        double num = 0.0+1000*(value+0.1)/0.9;
        ui->robot1_yaw_input->setText(QString::number(num));
    }
}
void Widget::robot1_locak(double value)
{
    if (value) {
        is_button = 0;  // 按钮按下时设置为2
    } else {
        is_button = 6;  // 按钮松开时设置为6
    }
    qDebug()<<"is_button:"<<is_button;
}
void Widget::shoubing_free_move(double value)
{
    if (value) {
        is_button = 1;  // 按钮按下时设置为2
    } else {
        is_button = 6;  // 按钮松开时设置为6
    }
    qDebug()<<"is_button:"<<is_button;
}
void Widget::shoubing_bulletmove(double value)
{
    if (value) {
        is_button = 4;  // 按钮按下时设置为2
    } else {
        is_button = 6;  // 按钮松开时设置为6
    }
    qDebug()<<"is_button:"<<is_button;
}

void Widget::shoubing_roll_open(double value)
{
    if (value) {
        is_button = 3;  // 按钮按下时设置为2
    } else {
        is_button = 6;  // 按钮松开时设置为6
    }
    qDebug()<<"is_button:"<<is_button;
}
void Widget::shoubing_dingshen(double value)
{
    if (value) {
        is_button = 2;  // 按钮按下时设置为2
    } else {
        is_button = 6;  // 按钮松开时设置为6
    }
    qDebug()<<"is_button:"<<is_button;
}
//################################################
//#                                              #
//#           界面初始化函数                      #
//#                                             #
//#                                              #
//################################################
void Widget::Widget_init()
{// 在Widget_init中添加
    /*ui->groupBox->setStyleSheet("QGroupBox::title {"
                                "background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
                                "stop:0 #00c6ff, stop:1 #0072ff);"
                                "padding: 2px 10px;"
                                "border-radius: 4px;"
                                "}");*/

    auto gamepads=QGamepadManager::instance()->connectedGamepads();
    m_gamepad=new QGamepad(*gamepads.begin(),this);
    // 设置 QML 文件路径
    ui->mapWidget->setSource(QUrl("qrc:/map.qml"));
    ui->mapWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);
    // 将 C++ 对象暴露给 QML
    ui->mapWidget->rootContext()->setContextProperty("mapHandler", this);
    ui->robot1_x_input->setText("0");
    ui->robot1_y_input->setText("0");
    ui->robot1_z_input->setText("500");
    ui->robot1_yaw_input->setText("0");
    // 创建新图表来显示所有数据
    m_axisX = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisX->setTitleText("Time");
    m_axisY->setTitleText("Value");
    m_axisX->setMin(0);
    m_axisY->setMin(AXIS_MIN_Y);  // 可以根据实际情况调整最小值
    m_axisX->setMax(AXIS_MAX_X);
    m_axisY->setMax(AXIS_MAX_Y);  // 根据三者的最大值来设置Y轴最大值

    // 创建合并后的图表对象
    m_chart_all = new QChart();
    m_chart_all->addAxis(m_axisX, Qt::AlignBottom);
    m_chart_all->addAxis(m_axisY, Qt::AlignLeft);

    // 创建三个折线数据系列
    m_lineSeries_X = new QLineSeries();
    m_lineSeries_X->setPointsVisible(true);
    m_lineSeries_X->setName("X");
    m_chart_all->addSeries(m_lineSeries_X);
    m_lineSeries_X->attachAxis(m_axisX);
    m_lineSeries_X->attachAxis(m_axisY);

    m_lineSeries_Y = new QLineSeries();
    m_lineSeries_Y->setPointsVisible(true);
    m_lineSeries_Y->setName("Y");
    m_chart_all->addSeries(m_lineSeries_Y);
    m_lineSeries_Y->attachAxis(m_axisX);
    m_lineSeries_Y->attachAxis(m_axisY);

    m_lineSeries_Z = new QLineSeries();
    m_lineSeries_Z->setPointsVisible(true);
    m_lineSeries_Z->setName("Z");
    m_chart_all->addSeries(m_lineSeries_Z);
    m_lineSeries_Z->attachAxis(m_axisX);
    m_lineSeries_Z->attachAxis(m_axisY);

    // 设置图表的动画效果
    m_chart_all->setAnimationOptions(QChart::SeriesAnimations);

    // 将图表设置到图形视图上
    ui->graphicsView_xyz->setChart(m_chart_all);
    ui->graphicsView_xyz->setRenderHint(QPainter::Antialiasing); // 设置抗锯齿



    // 创建新图表来显示所有数据
    m_axisX2 = new QValueAxis();
    m_axisY2 = new QValueAxis();
    m_axisX2->setTitleText("Time");
    m_axisY2->setTitleText("Value");
    m_axisX2->setMin(0);
    m_axisY2->setMin(AXIS_MIN_Y_roll);  // 可以根据实际情况调整最小值
    m_axisX2->setMax(AXIS_MAX_X);
    m_axisY2->setMax(std::max({AXIS_MAX_Y_roll, AXIS_MAX_Y_pitch, AXIS_MAX_Y_yaw}));  // 根据三者的最大值来设置Y轴最大值

    // 创建合并后的图表对象
    m_chart_alljiaodu = new QChart();
    m_chart_alljiaodu->addAxis(m_axisX2, Qt::AlignBottom);
    m_chart_alljiaodu->addAxis(m_axisY2, Qt::AlignLeft);

    // 创建三个折线数据系列
    m_lineSeries_roll = new QLineSeries();
    m_lineSeries_roll->setPointsVisible(true);
    m_lineSeries_roll->setName("Roll");
    m_chart_alljiaodu->addSeries(m_lineSeries_roll);
    m_lineSeries_roll->attachAxis(m_axisX2);
    m_lineSeries_roll->attachAxis(m_axisY2);

    m_lineSeries_pitch = new QLineSeries();
    m_lineSeries_pitch->setPointsVisible(true);
    m_lineSeries_pitch->setName("Pitch");
    m_chart_alljiaodu->addSeries(m_lineSeries_pitch);
    m_lineSeries_pitch->attachAxis(m_axisX2);
    m_lineSeries_pitch->attachAxis(m_axisY2);

    m_lineSeries_yaw = new QLineSeries();
    m_lineSeries_yaw->setPointsVisible(true);
    m_lineSeries_yaw->setName("Yaw");
    m_chart_alljiaodu->addSeries(m_lineSeries_yaw);
    m_lineSeries_yaw->attachAxis(m_axisX2);
    m_lineSeries_yaw->attachAxis(m_axisY2);

    // 设置图表的动画效果
    m_chart_alljiaodu->setAnimationOptions(QChart::SeriesAnimations);

    // 将图表设置到图形视图上
    ui->graphicsView_jiaodu->setChart(m_chart_alljiaodu);
    ui->graphicsView_jiaodu->setRenderHint(QPainter::Antialiasing); // 设置抗锯齿
    // 设置图表主题为深色
    m_chart_all->setTheme(QChart::ChartThemeDark);
    m_chart_alljiaodu->setTheme(QChart::ChartThemeDark);

    // 设置图表背景透明
    m_chart_all->setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));
    m_chart_alljiaodu->setBackgroundBrush(QBrush(QColor(0, 0, 0, 0)));

    // 设置坐标轴颜色
    QPen axisPen(QColor(100, 200, 255));
    axisPen.setWidth(2);
    m_axisX->setLinePen(axisPen);
    m_axisY->setLinePen(axisPen);
    m_axisX2->setLinePen(axisPen);
    m_axisY2->setLinePen(axisPen);

    // 设置坐标轴文字颜色
    m_axisX->setLabelsColor(QColor(180, 230, 255));
    m_axisY->setLabelsColor(QColor(180, 230, 255));
    m_axisX2->setLabelsColor(QColor(180, 230, 255));
    m_axisY2->setLabelsColor(QColor(180, 230, 255));

    // 设置标题颜色
    m_axisX->setTitleBrush(QBrush(QColor(180, 230, 255)));
    m_axisY->setTitleBrush(QBrush(QColor(180, 230, 255)));
    m_axisX2->setTitleBrush(QBrush(QColor(180, 230, 255)));
    m_axisY2->setTitleBrush(QBrush(QColor(180, 230, 255)));

    // 设置线条颜色和宽度
    m_lineSeries_X->setPen(QPen(QColor(0, 255, 255), 2.0));
    m_lineSeries_Y->setPen(QPen(QColor(0, 200, 255), 2.0));
    m_lineSeries_Z->setPen(QPen(QColor(0, 150, 255), 2.0));

    m_lineSeries_roll->setPen(QPen(QColor(255, 100, 0), 2.0));
    m_lineSeries_pitch->setPen(QPen(QColor(0, 255, 150), 2.0));
    m_lineSeries_yaw->setPen(QPen(QColor(150, 0, 255), 2.0));

    // 为关键控件添加阴影效果
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
    shadowEffect->setBlurRadius(15);
    shadowEffect->setColor(QColor(0, 200, 255, 150));
    shadowEffect->setOffset(3, 3);
    //ui->groupBox->setGraphicsEffect(shadowEffect);

    QGraphicsDropShadowEffect *chartShadow = new QGraphicsDropShadowEffect(this);
    chartShadow->setBlurRadius(10);
    chartShadow->setColor(QColor(0, 150, 255, 100));
    chartShadow->setOffset(2, 2);
    ui->graphicsView_xyz->setGraphicsEffect(chartShadow);
    //ui->graphicsView_jiaodu->setGraphicsEffect(chartShadow->clone());

    ui->mapWidget->setStyleSheet(
        "background: #2c3e50;"
        "border: 1px solid #7f8c8d;"
        "border-radius: 6px;"
        );


}
//################################################
//#                                              #
//#           运动数据包                    #
//#                                             #
//#                                              #
//################################################
void Widget::print_move_data()
{
    // 根据is_connect决定发送目标
    QString targetIp;
    quint16 targetPort;

    if (is_connect == 1) {
        ui->robot1_connect_state->setStyleSheet(
            "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00cc00, stop:1 #009900);"
            "border-radius: 10px;"
            "border: 1px solid #008000;"
            );
        ui->robot2_connect_state->setStyleSheet(
            "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6666, stop:1 #cc0000);"
            "border-radius: 10px;"
            "border: 1px solid #990000;"
            );
        targetIp="192.168.2.22";
        targetPort=14550;
    } else if (is_connect == 2) {
        ui->robot2_connect_state->setStyleSheet(
            "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #00cc00, stop:1 #009900);"
            "border-radius: 10px;"
            "border: 1px solid #008000;"
            );
        ui->robot1_connect_state->setStyleSheet(
            "background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ff6666, stop:1 #cc0000);"
            "border-radius: 10px;"
            "border: 1px solid #990000;"
            );
        targetIp="192.168.2.16";
        targetPort=14551;
    } else {
        // is_connect为0或其他值，不发送
        return;
    }
    QByteArray data_send;
    data_send.resize(22);
    data_send[0] = 0xFE;  //数据头
    data_send[1] = 0x23;  //消息ID
    data_send[2] = 0x11;  //数据长度：12
    //--------------平动相关-------------
    qint32 pingdong_x = (qint32)(ui->robot1_x_input->text().toDouble()*10000);
    qint32 pingdong_y = (qint32)(ui->robot1_y_input->text().toDouble()*10000);
    qint32 pingdong_z = (qint32)(ui->robot1_z_input->text().toDouble()*10000);
    //传入x轴平动速度
    data_send[3] = pingdong_x;
    data_send[4] = pingdong_x>>8;
    data_send[5] = pingdong_x>>16;
    data_send[6] = pingdong_x>>24;
    //传入y轴平动速度
    data_send[7] = pingdong_y;
    data_send[8] = pingdong_y>>8;
    data_send[9] = pingdong_y>>16;
    data_send[10] = pingdong_y>>24;
    //传入z轴平动速度
    data_send[11] = pingdong_z;
    data_send[12] = pingdong_z>>8;
    data_send[13] = pingdong_z>>16;
    data_send[14] = pingdong_z>>24;
    //--------------转动相关-------------
    qint32 zhuandong_yaw = (qint32)(ui->robot1_yaw_input->text().toDouble()*10000);
    //传入yaw转动速度
    data_send[15] = zhuandong_yaw;
    data_send[16] = zhuandong_yaw>>8;
    data_send[17] = zhuandong_yaw>>16;
    data_send[18] = zhuandong_yaw>>24;
    //异或校验
    data_send[19] = is_button;
    //帧尾
    data_send[20] = 0x01;
    data_send[21] = 0xFF;
    udp->sendData(data_send, targetIp, targetPort);
}
void Widget::sendPathToROV1()
{
    QString targetIp="192.168.2.22";
    quint16 targetPort=14550;
    QByteArray data_send;
    data_send.resize(5+waypoint_ROV1.size()*8);
    data_send[0] = 0xFE;  //数据头
    data_send[1] = 0x23;  //消息ID
    data_send[2] = 0x11;  //数据长度：12
    for(int i=0;i<waypoint_ROV1.size();i++)
    {
        longitude_ROV1=(qint32)(waypoint_ROV1[i].longitude()*10000);
        latitude_ROV1=(qint32)(waypoint_ROV1[i].latitude()*10000);
        data_send[i*8+3]=longitude_ROV1;
        data_send[i*8+4]=longitude_ROV1>>8;
        data_send[i*8+5]=longitude_ROV1>>16;
        data_send[i*8+6]=longitude_ROV1>>24;
        data_send[i*8+7]=latitude_ROV1;
        data_send[i*8+8]=latitude_ROV1>>8;
        data_send[i*8+9]=latitude_ROV1>>16;
        data_send[i*8+10]=latitude_ROV1>>24;
    }
    data_send[(waypoint_ROV1.size()-1)*8+11]=0x01;
    data_send[(waypoint_ROV1.size()-1)*8+12]=0xFF;
    udp->sendData(data_send, targetIp, targetPort);
}
//################################################
//#                                              #
//#           处理函数                    #
//#                                             #
//#                                              #
//################################################
void Widget::handleUdpData1(const QByteArray &data)
{
    // 处理来自机器人1 (192.168.2.22) 的数据
    qDebug() << "Received data from Robot A (192.168.2.22), size:" << data.size();
    for(int i = 0; i < data.length(); i++)
    {
        qDebug() << "Byte" << i << ":" << QString::number(static_cast<uchar>(data.at(i)), 16).rightJustified(2, '0');
        if((unsigned char)data.at(i) != 0xFE) continue;
        if((unsigned char)data.at(i+1) == 0x00) unpack_robot1_zero(data,i);
    }

}
void Widget::handleUdpData2(const QByteArray &data)
{
    // 处理来自机器人2 (192.168.2.16) 的数据
    qDebug() << "Received data from Robot B (192.168.2.16), size:" << data.size();
    for(int i = 0; i < data.length()-2 ; i++)
    {

        if((unsigned char)data.at(i) != 0xFE) continue;
        if((unsigned char)data.at(i+1) == 0x00) unpack_robot2_zero(data,i);
    }
}
void Widget::handleBuoy1Data(const QByteArray &data)
{
    qDebug() << "Received data from Robot A (192.168.2.25), size:" << data.size();
    for(int i = 0; i < data.length(); i++)
    {
        qDebug() << "Byte" << i << ":" << QString::number(static_cast<uchar>(data.at(i)), 16).rightJustified(2, '0');
        if((unsigned char)data.at(i) != 0xFE) continue;
        if((unsigned char)data.at(i+1) == 0x25) unpack_Buoy1_zero(data,i);
    }

}

void Widget::handleBuoy2Data(const QByteArray &data)
{
    qDebug() << "Received data from Robot A (192.168.2.19), size:" << data.size();
    for(int i = 0; i < data.length(); i++)
    {
        qDebug() << "Byte" << i << ":" << QString::number(static_cast<uchar>(data.at(i)), 16).rightJustified(2, '0');
        if((unsigned char)data.at(i) != 0xFE) continue;
        if((unsigned char)data.at(i+1) == 0x19) unpack_Buoy2_zero(data,i);
    }
}
void Widget::unpack_robot1_zero(QByteArray data_hear,int head)
{
    int data_length = (int)data_hear.at(head+2);
    if(data_length!=36) return;
    if(data_length+5+head != data_hear.length())
    {    qDebug()<<"unpack_robot1_zero error";
        return;}
    //更新数据读取head
    head = head + 3;
    unsigned char get_byte[8];
    //水深，水温，水压，水质QByteArray data_hear,int head
    for(int cnt = 0 ; cnt <= 1 ; cnt++){
        for (int i = 0 ;i <= 3 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        water_state[cnt] = (float)(get_byte[0]+(get_byte[1]<<8)+(get_byte[2]<<16)+(get_byte[3]<<24))/10000.0;
    }
    //1-8号推进器PWM波
    for(int cnt = 0 ; cnt <= 7 ; cnt++){
        for (int i = 0 ;i <= 1 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        pwm[cnt] = (uint16_t)(get_byte[0]+(get_byte[1]<<8))/10000.0;
    }
    //roll,yaw,pitch
    for(int cnt = 0 ; cnt <= 2 ; cnt++){
        for (int i = 0 ;i <= 3 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        angle_data[cnt] = (float)(get_byte[0]+(get_byte[1]<<8)+(get_byte[2]<<16)+(get_byte[3]<<24))/10000.0;
    }
     ui->rov1_depth_lineEdit->setText(QString("%4").arg(water_state[0]));
    ui->rov1_roll_lineEdit->setText(QString("%4").arg(angle_data[0]));
    ui->rov1_pitch_lineEdit->setText(QString("%4").arg(angle_data[1]));
    ui->rov1_yaw_lineEdit->setText(QString("%4").arg(angle_data[2]));

}

void Widget::unpack_robot2_zero(QByteArray data_hear,int head)
{
    int data_length = (int)data_hear.at(head+2);
    if(data_length!=36) return;
    if(data_length+5+head != data_hear.length())
    {    qDebug()<<"unpack_robot1_zero error";
        return;}
    //更新数据读取head
    head = head + 3;
    unsigned char get_byte[8];
    //水深，水温，水压，水质
    for(int cnt = 0 ; cnt <= 1 ; cnt++){
        for (int i = 0 ;i <= 3 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        water_state[cnt] = (float)(get_byte[0]+(get_byte[1]<<8)+(get_byte[2]<<16)+(get_byte[3]<<24))/10000.0;
    }
    //1-8号推进器PWM波
    for(int cnt = 0 ; cnt <= 7 ; cnt++){
        for (int i = 0 ;i <= 1 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        pwm[cnt] = (uint16_t)(get_byte[0]+(get_byte[1]<<8))/10000.0;
    }
    //roll,yaw,pitch
    for(int cnt = 0 ; cnt <= 2 ; cnt++){
        for (int i = 0 ;i <= 3 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        angle_data[cnt] = (float)(get_byte[0]+(get_byte[1]<<8)+(get_byte[2]<<16)+(get_byte[3]<<24))/10000.0;
    }
    ui->rov2_depth_lineEdit->setText(QString("%4").arg(water_state[0]));
    ui->rov2_roll_lineEdit->setText(QString("%4").arg(angle_data[0]));
    ui->rov2_pitch_lineEdit->setText(QString("%4").arg(angle_data[1]));
    ui->rov2_yaw_lineEdit->setText(QString("%4").arg(angle_data[2]));
}

void Widget::unpack_Buoy1_zero(QByteArray data_hear, int head)
{
    int data_length = (int)data_hear.at(head+2);
    if(data_length!=8) return;
    if(data_length+5+head != data_hear.length())
    {    qDebug()<<"unpack_robot1_zero error";
        return;}
    //更新数据读取head
    head = head + 3;
    unsigned char get_byte[8];
    //水深，水温，水压，水质
    for(int cnt = 0 ; cnt <= 1 ; cnt++){
        for (int i = 0 ;i <= 3 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        GPS_Buoy1_state[cnt] = (float)(get_byte[0]+(get_byte[1]<<8)+(get_byte[2]<<16)+(get_byte[3]<<24))/10000.0;
    }
    /*
    通过经纬度结算牛耕式遍历
*/
    //udp->sendData()
}

void Widget::unpack_Buoy2_zero(QByteArray data_hear, int head)
{
    int data_length = (int)data_hear.at(head+2);
    if(data_length!=8) return;
    if(data_length+5+head != data_hear.length())
    {    qDebug()<<"unpack_robot1_zero error";
        return;}
    //更新数据读取head
    head = head + 3;
    unsigned char get_byte[8];
    //水深，水温，水压，水质
    for(int cnt = 0 ; cnt <= 1 ; cnt++){
        for (int i = 0 ;i <= 3 ; i++){
            get_byte[i] = (unsigned char)data_hear.at(head);
            head++;
        }
        GPS_Buoy2_state[cnt] = (float)(get_byte[0]+(get_byte[1]<<8)+(get_byte[2]<<16)+(get_byte[3]<<24))/10000.0;
    }
}
void Widget::handleOtherUdpData(const QByteArray &data, const QString &senderIp)
{
    // 处理来自其他来源的数据
    qDebug() << "Received data from unknown source:" << senderIp << ", size:" << data.size();

    // 可以根据需要添加处理逻辑，或者只是记录日志
}
// 在 updateCameraView 函数中添加深度信息显示
// 修改updateCameraView函数
void Widget::updateCameraView(QImage leftImage, const QString &senderIp)
{
    // 根据发送者IP选择显示位置
    if (senderIp == "192.168.2.24") { // 第一个ROV
        QPixmap pixmap = QPixmap::fromImage(leftImage)
                         .scaled(ui->label_camera_view->width(),
                                 ui->label_camera_view->height(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        ui->label_camera_view->setPixmap(pixmap);

        if (depthInfoMap.contains(senderIp)) {
            float depth = depthInfoMap[senderIp];
            ui->label_depth_info1->setText(QString("深度: %1 米").arg(depth, 0, 'f', 2));
        } else {
            ui->label_depth_info1->setText("深度: 等待数据...");
        }
    } 
    else if (senderIp == "192.168.2.18") { // 第二个ROV
        QPixmap pixmap = QPixmap::fromImage(leftImage)
                         .scaled(ui->label_camera_view_2->width(),
                                 ui->label_camera_view_2->height(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
        ui->label_camera_view_2->setPixmap(pixmap);

        if (depthInfoMap.contains(senderIp)) {
            float depth = depthInfoMap[senderIp];
            ui->label_depth_info2->setText(QString("深度: %1 米").arg(depth, 0, 'f', 2));
        } else {
            ui->label_depth_info2->setText("深度: 等待数据...");
        }
    }

    // 更新帧率统计
    static QMap<QString, int> frameCounters;
    static QMap<QString, qint64> lastFrameTime;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();

    if (!lastFrameTime.contains(senderIp)) {
        frameCounters[senderIp] = 0;
        lastFrameTime[senderIp] = currentTime;
    }

    frameCounters[senderIp]++;
}

// 在 updateDepthInfo 函数中存储深度信息
void Widget::updateDepthInfo(float depth, const QString &senderIp)
{
    // 存储深度信息
    depthInfoMap[senderIp] = depth;

    // 如果当前显示的是这个发送者的图像，立即更新深度信息
    if (senderIp == "192.168.2.24") {
        ui->label_depth_info1->setText(QString("深度: %1 米").arg(depth));
    } 
    else if (senderIp == "192.168.2.18") {
        ui->label_depth_info2->setText(QString("深度: %1 米").arg(depth));
    }

    qDebug() << "Depth info updated for" << senderIp << ":" << depth;
}



/*void Widget::handleFloatFeedback(const QVector<float> &feedback, const QString &senderIp)
{
    if (feedback.size() >= 4) {
        floatFeedbackMap[senderIp] = feedback;
        qDebug() << "Stored float feedback from" << senderIp << ":"
                 << feedback[0] << feedback[1] << feedback[2] << feedback[3];
    }
}*/
//################################################
//#                                              #
//#           跟踪函数                     #
//#                                             #
//#                                              #
//################################################
void Widget::updateTargetPosition(quint16 x, quint16 y, const QString &senderIp)
{
    // 只处理来自192.168.2.24的视频流
    if (senderIp == "192.168.2.24") {
        // 将目标位置转发给tracking模块
        QMetaObject::invokeMethod(m_tracking, "updateTargetPosition", Qt::QueuedConnection,
                                  Q_ARG(quint16, x),
                                  Q_ARG(quint16, y));
    }
}
void Widget::on_pushButton_track_clicked()
{
    static bool trackingActive = false;
    trackingActive = !trackingActive;

    if (trackingActive) {
        // 使用线程安全方式启动跟踪
        QMetaObject::invokeMethod(m_tracking, "startTracking", Qt::QueuedConnection);
        ui->Pushutton_track->setText("停止跟踪");
    } else {
        // 使用线程安全方式停止跟踪
        QMetaObject::invokeMethod(m_tracking, "stopTracking", Qt::QueuedConnection);
        ui->Pushutton_track->setText("开始跟踪");
    }
}

void Widget::handleTrackingCommand(float yawSpeed, float moveSpeed)
{
    // 设置控制量到对应的输入框
    ui->robot1_yaw_input->setText(QString::number(yawSpeed));
    ui->robot1_x_input->setText(QString::number(moveSpeed));

    // 在UI上显示当前控制量
    //ui->label_track_yaw->setText(QString("Yaw: %1").arg(yawSpeed));
    //ui->label_track_speed->setText(QString("Speed: %1").arg(moveSpeed));
}

void Widget::updateTrackingVisual(QImage processedImage)
{
    // 显示处理后的图像
    QPixmap pixmap = QPixmap::fromImage(processedImage)
                         .scaled(ui->tabWidget->width(),
                                 ui->tabWidget->height(),
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);
    ui->label_tracking_view->setPixmap(pixmap);
}

//################################################
//#                                              #
//#           界面夕狗函数                     #
//#                                             #
//#                                              #
//################################################
Widget::~Widget()
{
    if (m_trackingThread && m_trackingThread->isRunning()) {
        // 请求停止跟踪
        QMetaObject::invokeMethod(m_tracking, "stopTracking", Qt::BlockingQueuedConnection);

        // 退出线程
        m_trackingThread->quit();
        m_trackingThread->wait();
    }

    // 删除跟踪对象 - 必须在线程停止后
    delete m_tracking;

    // 其他资源清理...
    delete ui;
}





