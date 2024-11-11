#include "Monitoring.h"

Monitoring::Monitoring(QWidget *parent) : QMainWindow(parent)
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
	//QObject::connect(ui.btnStart, &QPushButton::clicked, this, &Monitoring::Start);

	QPushButton *aboutButton = new QPushButton("О программе");
	aboutButton->setShortcut(QKeySequence("CTRL + O"));
	QObject::connect(aboutButton, &QPushButton::clicked, this, &Monitoring::AboutProgramm);
	ui.toolBar->addWidget(aboutButton);// Программа монитор программноаппаратного комплекса УПП-1 (вер.. ид)

	ui.btnDisconnect->setEnabled(false);
	ui.btnMonitor->setEnabled(false);
	ui.btnStop->setEnabled(false);
	//ui.btnStart->setEnabled(false);
	ui.ChangeSettings->setEnabled(false);

	timer->setInterval(1.f);
	connect(timer, SIGNAL(timeout()), this, SLOT(Update()));

	/*timerAverage->setInterval(1.f);
	connect(timerAverage, SIGNAL(timeout()), this, SLOT(Average()));*/
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
	//ui.btnStart->setEnabled(true);
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

//void Monitoring::Start()
//{
//	if (!work->port->isOpen())return;
//	int ratio = pow(2, ui.spd->currentIndex());
//	int step = ui.step->toPlainText().toInt();
//	work->ReadyStart(0, ratio, 200, step);
//	timer->start();
//	_sleep(10);
//	work->processRun();
//}

void Monitoring::StepThreading()
{
	ConvertAngle(work->measure->refAngle);
	ui.textAngle->setPlainText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + to_string(sec) + "'" + "'"));
	ui.I->setPlainText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.RMS)));
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


void Monitoring::AboutProgramm()
{
	QMessageBox box;
	box.setText("Программа монитор программноаппаратного комплекса УПП-1 \nFirmware версия 1.0 \nИдентификационный номер (CRC32): 889DBC3C");
	box.exec();
}

vector<Data> Monitoring::EraseErrors(vector<Data> cont)
{
	vector<Data> temp;
	temp.push_back(cont[0]);
	for (int i = 1; i < cont.size(); i++)
	{
		if (abs(temp[temp.size() - 1].V1 - cont[i].V1) < 60);
		temp.push_back(cont[i]);
	}
	vector<Data> result;
	for (int i = 3; i < temp.size() - 2; i++)
	{

		float val1 = (temp[i - 1].V1 + temp[i - 2].V1) / 2;
		float val2 = (temp[i + 1].V1 + temp[i + 2].V1) / 2;
		float delta = (val1 + val2) / 10;
		if (val1 < 50)delta = 5;
		if (abs(temp[i].V1 - val1) < delta || abs(temp[i].V1 - val2) < delta)
			result.push_back(temp[i]);
	}
	return result;
}

Data Monitoring::Read()
{
	Data temp;

	temp.angle = angle;
	temp.min = min;
	temp.sec = sec;
	temp.angleGrad = work->measure->refAngle;
	temp.V1 = measures_t(work->measure->Result).V1.RMS;
	temp.V2 = measures_t(work->measure->Result).V2.RMS;
	temp.Phase1 = measures_t(work->measure->Result).V1.Phase;
	temp.Phase2 = measures_t(work->measure->Result).V2.Phase;
	temp.I = measures_t(work->measure->Result).I.RMS;
	temp.IPhase = measures_t(work->measure->Result).I.Phase;

	return temp;
}

vector<Data> Monitoring::AverageData(vector<Data> cont)
{
	vector<Data> temp;
	int* Angle = VectorInMass(cont, false);
	int* Min = VectorInMass(cont, true);
	float* V = VectorInMassV(cont, true);
	float* V2 = VectorInMassV(cont, false);
	for (int i = 0; i < cont.size(); i++)
	{
		int currentMin = Min[i];
		int currentAngle = Angle[i];
		if (Contains(temp, currentAngle, currentMin))continue;
		float sum = 0;
		float sum2 = 0;
		float sumI = 0;
		float sumIP = 0;
		int k = 0;
		for (int j = i; j < cont.size(); j++)
		{
			if (currentAngle == Angle[j] && currentMin == Min[j])
			{
				sum += V[j];
				sum2 += V2[j];
				sumI += cont[j].I;
				sumIP += cont[j].IPhase;
				k++;
			}
		}
		sum /= k;
		sum2 /= k;
		sumI /= k;

		Data dt;
		dt.angle = currentAngle;
		dt.min = currentMin;

		dt.V1 = sum;
		dt.V2 = sum2;
		dt.I = sumI;
		dt.IPhase = sumIP;

		temp.push_back(dt);
	}
	return temp;
}

int* Monitoring::VectorInMass(vector<Data> cont, bool mins)
{
	int* temp = new int[cont.size()];
	for (int i = 0; i < cont.size(); i++)
	{
		if (mins)
			temp[i] = cont[i].min;
		else
			temp[i] = cont[i].angle;
	}
	return temp;
}

float* Monitoring::VectorInMassV(vector<Data> cont, bool V1)
{
	float* V = new float[cont.size()];
	for (int i = 0; i < cont.size(); i++)
	{
		if (V1)V[i] = cont[i].V1;
		else V[i] = cont[i].V2;
	}
	return V;
}

bool Monitoring::Contains(vector<Data> cont, int _angle, int _min)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].min == _min)
			if (cont[i].angle == _angle)
				return true;
	}
	return false;
}

void Monitoring::Average()
{
	//data.push_back(Read());
}

void Monitoring::Update()
{
	//data = EraseErrors(data);
	//data = AverageData(data);
	StepThreading();
	if (!work->port->isOpen())Disconnect();
	//data.clear();
}