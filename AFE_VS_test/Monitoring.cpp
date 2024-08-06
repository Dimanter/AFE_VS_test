#include "Monitoring.h"

Monitoring::Monitoring(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	connection = false;
	work = new Work("Changed", 2400, 1400);
	connect(work, &Work::portStatus, this, &Monitoring::StatusConnect);
	Create();
}



Monitoring::~Monitoring()
{}

void Monitoring::Create()
{
	Refresh();
	QObject::connect(ui.btnRefresh, &QPushButton::clicked, this, &Monitoring::Refresh);
	QObject::connect(ui.btnConnect, &QPushButton::clicked, this, &Monitoring::Connect);
	QObject::connect(ui.btnDisconnect, &QPushButton::clicked, this, &Monitoring::Disconnect);
	QObject::connect(ui.btnMonitor, &QPushButton::clicked, this, &Monitoring::Monitor);
	QObject::connect(ui.btnStop, &QPushButton::clicked, this, &Monitoring::Stop);
	QObject::connect(ui.ChangeSettings, &QPushButton::clicked, this, &Monitoring::ChangeSettings);

	ui.btnDisconnect->setEnabled(false);
	ui.btnMonitor->setEnabled(false);
	ui.btnStop->setEnabled(false);
	ui.ChangeSettings->setEnabled(false);

	timer->setInterval(100.f);
	connect(timer, SIGNAL(timeout()), this, SLOT(Update()));
}

void Monitoring::Refresh()
{
	ui.comboBox->clear();
	const auto serialPortInfos = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& portInfo : serialPortInfos)
	{
		ui.comboBox->insertItem(portInfo.productIdentifier(), portInfo.portName());
	}
}

void Monitoring::Connect()
{
	if (!work->Connect(ui.comboBox->currentText()))return;
	connection = true;
	ui.btnConnect->setEnabled(false);
	ui.btnDisconnect->setEnabled(true);
	ui.btnMonitor->setEnabled(true);
	ui.btnStop->setEnabled(true);
	ui.ChangeSettings->setEnabled(true);
	ui.StatusCon->setText(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));

	work->Stop();
}

void Monitoring::Disconnect()
{
	work->Stop();
	work->Disconnect();
	connection = false;
	ui.btnConnect->setEnabled(true);
	ui.btnDisconnect->setEnabled(false);
	ui.btnMonitor->setEnabled(false);
	ui.btnStop->setEnabled(false);
	ui.ChangeSettings->setEnabled(false);
	ui.StatusCon->setText(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));
	timer->stop();
}

void Monitoring::Stop()
{
	work->Stop();
	ui.ChangeSettings->setEnabled(true);
}

void Monitoring::StepThreading()
{
	ConvertAngle(work->measure->refAngle);
	ui.textAngle->setPlainText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + to_string(sec) + "'" + "'"));
	ui.I->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.RMS)));
	//ui.V2Phase->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.Phase)));
	//ui.V1Phase->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.Phase)));
	ui.V1->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.RMS)));
	ui.V2->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.RMS)));
	ui.IPhase->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.Phase)));
}

void Monitoring::Monitor()
{
	if (!work->port->isOpen())return;
	timer->start();
	work->Monitor();
	ui.ChangeSettings->setEnabled(false);
}

void Monitoring::StatusConnect(bool connected)
{
	ui.btnConnect->setEnabled(!connected);
}

void Monitoring::ConvertAngle(float _angle)
{
	float grad, _min, _sec;
	grad = std::trunc(_angle);
	_min = _angle - grad;
	_min = std::trunc(_min * 60.0);
	_sec = (_angle - grad) * 60.0 - _min;
	_sec = std::trunc(_sec * 60.0);
	angle = grad;
	min = _min;
	sec = _sec;
}

void Monitoring::ChangeSettings()
{
	try 
	{
		float freq = ui.frequency->value();
		float res = ui.resist->value();
		work->ChangeSettings(freq, res);
		ui.ChangeSettings->setEnabled(false);
	}
	catch (...)
	{

	}
	
}

void Monitoring::Update()
{
	StepThreading();
	if (!work->port->isOpen())Disconnect();
}