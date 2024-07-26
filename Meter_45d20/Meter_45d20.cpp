#include "Meter_45d20.h"

Meter::Meter(QWidget* parent)
{
	QPalette darkPalette;//Цветовая палитра приложения
	darkPalette.setColor(QPalette::Window, QColor(40, 40, 40));
	darkPalette.setColor(QPalette::WindowText, QColor(255, 255, 255));
	darkPalette.setColor(QPalette::Base, QColor(75, 75, 75));
	darkPalette.setColor(QPalette::AlternateBase, QColor(75, 75, 75));
	darkPalette.setColor(QPalette::ToolTipBase, QColor(75, 75, 75));
	darkPalette.setColor(QPalette::ToolTipText, QColor(255, 255, 255));
	darkPalette.setColor(QPalette::Text, QColor(255, 255, 255));
	darkPalette.setColor(QPalette::Button, QColor(50, 50, 50));
	darkPalette.setColor(QPalette::ButtonText, QColor(255, 255, 255));
	darkPalette.setColor(QPalette::BrightText, QColor(80, 80, 80));
	darkPalette.setColor(QPalette::Link, QColor(75, 75, 75));
	darkPalette.setColor(QPalette::Highlight, QColor(127, 133, 192));
	darkPalette.setColor(QPalette::HighlightedText, QColor(255, 192, 255));

	CreateWindow();
	window->show();

	qApp->setPalette(darkPalette);
	QApplication::exec();
}

void Meter::CreateWindow()
{
	//Main window ================================================================================
	window->setWindowTitle("Meter СКТ-232Б");
	QLabel* label = new QLabel("Порт ");

	QFont font = label->font();
	font.setPixelSize(40);

	comboPort1 = new QComboBox;
	comboPort1->setStyleSheet("QComboBox{"
		"background-color: #323232;"
		"color: #CCCCCC;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 5px;"
		"border-color: #CCCCCC;"
		"font: bold 14px;"
		"}"
		"QComboBox:editable{"
		"background: #CCCCCC;"
		"}");
	RefreshComboBox();


	btnRefreshPort = new QPushButton("Обновить");
	QObject::connect(btnRefreshPort, &QPushButton::clicked, this, &Meter::RefreshComboBox);
	btnRefreshPort->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #CCCCCC;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	QPushButton* btnReport = new QPushButton("Создать отчёт");
	QObject::connect(btnReport, &QPushButton::clicked, this, &Meter::Start);
	btnReport->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #CCCCCC;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	btnConnect = new QPushButton("Подключить");
	QObject::connect(btnConnect, &QPushButton::clicked, this, &Meter::Connect);
	btnConnect->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #CCCCCC;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	btnDisconnect = new QPushButton("Отключить");
	QObject::connect(btnDisconnect, &QPushButton::clicked, this, &Meter::Disconnect);
	btnDisconnect->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #CCCCCC;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	QPushButton* btnMonitor = new QPushButton("Монитор");
	QObject::connect(btnMonitor, &QPushButton::clicked, this, &Meter::StartMonitor);
	btnMonitor->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #00C2DF;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #00C2DF;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	QObject::connect(btnStart, &QPushButton::clicked, this, &Meter::Start);


	btnStop = new QPushButton("Стоп");
	QObject::connect(btnStop, &QPushButton::clicked, this, &Meter::Stop);
	btnStop->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #F44336;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #F44336;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	//StatusCon = new QLabel(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));

	QVBoxLayout* PortV = new QVBoxLayout();
	QVBoxLayout* StartV = new QVBoxLayout();
	QVBoxLayout* ConnectV = new QVBoxLayout();
	QVBoxLayout* AngleV = new QVBoxLayout();
	QVBoxLayout* RmsV = new QVBoxLayout();
	QVBoxLayout* IV = new QVBoxLayout();

	QHBoxLayout* PortH = new QHBoxLayout();
	QHBoxLayout* MeasureH = new QHBoxLayout();
	QHBoxLayout* ConnectH = new QHBoxLayout();

	QHBoxLayout* layoutMainHor = new QHBoxLayout();
	QVBoxLayout* layoutMainVer = new QVBoxLayout();

	//PortH->addWidget(label);
	//PortH->addWidget(comboPort1);

	//PortV->addLayout(PortH);
	/*PortV->addWidget(btnConnect);
	PortV->addWidget(btnDisconnect);
	PortV->addWidget(btnMonitor);
	PortV->addWidget(btnStart);
	PortV->addWidget(btnStop);*/

	window->setLayout(PortV);
}

void Meter::RefreshComboBox()
{
	comboPort1->clear();
	const auto serialPortInfos = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& portInfo : serialPortInfos)
	{
		comboPort1->insertItem(portInfo.productIdentifier(), portInfo.portName());
	}
}

void Meter::Start()
{
}

void Meter::Connect()
{
}

void Meter::Disconnect()
{
}

void Meter::Stop()
{
}

void Meter::StartMonitor()
{
}

Meter::~Meter()
{}
