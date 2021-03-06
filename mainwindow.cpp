#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

void MainWindow::creatChart()
{
    chartView = new QChartView(this);
    chart = chartView->chart();

    chart->setTitle("Data Display Chart");
    chartView->setChart(chart);
    this->setCentralWidget(chartView);

    series0 = new QLineSeries();
    series0->setName("Data");

    chart->addSeries(series0);

    axisX = new QValueAxis;
    axisX->setRange(0,100);

    axisX->setLabelFormat("%d"); //图例的格式  %d为十进制显示
    axisX->setTitleText("times");

    axisY = new QValueAxis;
    axisY->setRange(0,10);

    axisY->setLabelFormat("%d"); //图例的格式  %d为十进制显示
    axisY->setTitleText("value");

    chart->addAxis(axisY,Qt::AlignLeft);
    chart->addAxis(axisX, Qt::AlignBottom);

    series0->attachAxis(axisX);                           //把数据添加到坐标轴上
    series0->attachAxis(axisY);

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    bool Start = false;
    //创建串口对象

    connect(ui->USART,&QToolButton::clicked,[=,&Start](){
        QStringList m_serialPortName;
        foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
        {
            m_serialPortName << info.portName();
            qDebug()<<"serialPortName:"<<info.portName();
            serial.setPortName(info.portName());
        }
        //设置波特率
        serial.setBaudRate(QSerialPort::Baud115200);
        //设置数据位数
        serial.setDataBits(QSerialPort::Data8);
        //设置奇偶校验
        serial.setParity(QSerialPort::NoParity);
        //设置停止位
        serial.setStopBits(QSerialPort::OneStop);
        //设置流控制
        serial.setFlowControl(QSerialPort::NoFlowControl);

        if(!serial.open(QIODevice::ReadWrite))
        {
            qDebug()<<"USART OPEN FAIL";
            ui->lineEdit->setText("USART OPEN FAIL");
             Start = false;
            return ;
        }
        ui->lineEdit->setText("USART OPEN SUCCESSFUL");
        Start = true;
    });

    connect(ui->OpenUsart,&QToolButton::clicked,[=,&Start](){
        if(Start==true)
        {
            creatChart();
            connect(&serial, &QSerialPort::readyRead, this, &MainWindow::serialPort_readyRead);
        }
    });
}

void MainWindow::serialPort_readyRead()
{
    //从接收缓冲区中读取数据
    qreal  buf,y;
    static int t=0;
    static qreal old_y, new_y;
    QByteArray  buffer = serial.readAll();
    bool end = buffer.endsWith("&");

    if( !buffer.isEmpty() && end == true)
    {
        buffer.chop(1);
        buf = buffer.toFloat();
        //从界面中读取以前收到的数据
        qDebug()<<"Data:"<<buf<<endl;

        cur_x_min = axisX->min();
        cur_x_max = axisX->max();

        cur_y_min = axisY->min();
        cur_y_max = axisY->max();

        new_y=buf;
        series0->append(t,new_y);

        t+=1;

        if(t >=cur_x_max )
        {
            axisX->setRange(cur_x_min + 1, cur_x_max + 1);//图形向左平移1
        }
        if(new_y >=cur_y_max)
        {
            y = new_y-old_y;
            axisY->setRange(cur_y_min, cur_y_max + y);//图形向左平移1
        }
        else if(new_y<=cur_y_min)
        {
             y = old_y-new_y;
             axisY->setRange(cur_y_min - y, cur_y_max);//图形向左平移1
        }
        old_y=new_y;

        buffer.clear();
        serial.clear();
    }
    else
    {
        qDebug()<<"Data Type Fail";
        buffer.clear();
        serial.clear();
        return ;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

