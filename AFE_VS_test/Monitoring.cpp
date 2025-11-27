#include "Monitoring.h"

Monitoring::Monitoring(QWidget *parent) : QMainWindow(parent)
{
	ui.setupUi(this);
	connection = false;
	work = new Work("Changed", 2400, 1400);
	connect(work, &Work::portStatus, this, &Monitoring::StatusConnect);
	Create();
	ui.frequency->setSingleStep(100);
	ui.resist->setSingleStep(100);
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

	timer->setInterval(10.f);
	connect(timer, SIGNAL(timeout()), this, SLOT(Update()));

	timerAverage->setInterval(1000.f);
	connect(timerAverage, SIGNAL(timeout()), this, SLOT(Average()));
	//QApplication::exec();
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
	timerAverage->stop();
}

void Monitoring::Stop()
{
	work->Stop();
	timer->stop();
	timerAverage->stop();
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
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(1) << secDec;
	std::string roundedString = stream.str();
	ui.textAngle->setPlainText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + roundedString + "'" + "'"));

	std::ostringstream stream1;
	stream1 << std::fixed << std::setprecision(1) << measures_t(work->measure->Result).I.RMS;
	roundedString = stream1.str();
	ui.I->setPlainText(QString::fromStdString(roundedString));

	std::ostringstream stream2;
	stream2 << std::fixed << std::setprecision(1) << measures_t(work->measure->Result).V1.RMS;
	roundedString = stream2.str();
	ui.V1->setPlainText(QString::fromStdString(roundedString));

	std::ostringstream stream3;
	stream3 << std::fixed << std::setprecision(1) << measures_t(work->measure->Result).V2.RMS;
	roundedString = stream3.str();
	ui.V2->setPlainText(QString::fromStdString(roundedString));

	std::ostringstream stream4;
	stream4 << std::fixed << std::setprecision(1) << measures_t(work->measure->Result).I.Phase;
	roundedString = stream4.str();
	ui.IPhase->setPlainText(QString::fromStdString(roundedString));
}

void Monitoring::Monitor()
{
	if (!work->port->isOpen())return;
	timer->start();
	timerAverage->start();
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
	_sec = _sec * 60.0;//std::trunc(_sec * 60.0);
	angle = grad;
	min = _min;
	sec = _sec;
	secDec = std::round(_sec * 10.0f) / 10.0f;
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

int Monitoring::FindMinV(vector<Data> cont)
{
	float min = cont[0].V2;
	int idx = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		if (min > cont[i].V2)
		{
			min = cont[i].V2;
			idx = i;
		}
	}
	return idx;
}
vector<Data> Monitoring::EraseErrors(vector<Data> cont)
{
	vector<Data> result;
	int temp = FindMinV(cont);
	for (int i = 3; i < cont.size() - 2; i++)
	{
		float val1 = (cont[i - 1].V2 + cont[i - 2].V2) / 2;
		float val2 = (cont[i + 1].V2 + cont[i + 2].V2) / 2;
		float delta = (val1 + val2) / 20;

		if (val1 < 50)delta = 5;
		if (abs(cont[i].V2 - val1) < delta || abs(cont[i].V2 - val2) < delta)
			result.push_back(cont[i]);
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
	temp.V2 = measures_t(work->measure->Result).V2.RMS * 2;
	temp.Phase1 = measures_t(work->measure->Result).V1.Phase;
	temp.Phase2 = measures_t(work->measure->Result).V2.Phase;
	temp.I = measures_t(work->measure->Result).I.RMS;
	temp.IPhase = measures_t(work->measure->Result).I.Phase;

	return temp;
}

Data Monitoring::AverageData(vector<Data> &dataVector)
{
	Data result{};
	float U1 = 0;
	float U2 = 0;
	float I1 = 0;
	float I2 = 0;
	for (int i = 0; i < dataVector.size(); i++)
	{
		U1 += dataVector[i].V1;
		U2 += dataVector[i].V2;
		I1 += dataVector[i].I;
		I2 += dataVector[i].IPhase;
	}
	if (dataVector.size() > 1)
	{
		result.V1 = U1 / dataVector.size();
		result.V2 = U2 / dataVector.size();
		result.I = I1 / dataVector.size();
		result.IPhase = I2 / dataVector.size();
		result.angleGrad = dataVector[dataVector.size() - 1].angleGrad;
		result.angle = dataVector[dataVector.size() - 1].angle;
	}

	return result;
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

void Monitoring::Serialization(string file, vector<Data> cont)
{
	std::ofstream outFile(file + ".txt");
	if (!outFile) return; // Проверка открытия файла

	for (const auto& item : cont)
	{
		outFile << item.angle << " " << item.min << "'\t\t" << item.V2 << "\n";
	}
	outFile.close();
}

void Monitoring::Average()
{
	Serialization("data", data);
	
	data = EraseErrors(data);
	Data temp = AverageData(data);
	Serialization("data_0", data);
	data.clear();

	AvCont.push_back(temp);
	if (AvCont.size() > 2)
	{
		auto it = AvCont.begin();
		AvCont.erase(it);
	}
		
	temp = AverageData(AvCont);

	ConvertAngle(temp.angleGrad);
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(1) << secDec;
	std::string roundedString = stream.str();
	ui.textAngle->setPlainText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + roundedString + "'" + "'"));

	std::ostringstream stream1;
	stream1 << std::fixed << std::setprecision(1) << temp.I;
	roundedString = stream1.str();
	ui.I->setPlainText(QString::fromStdString(roundedString));

	std::ostringstream stream2;
	stream2 << std::fixed << std::setprecision(1) << temp.V1;
	roundedString = stream2.str();
	ui.V1->setPlainText(QString::fromStdString(roundedString));

	std::ostringstream stream3;
	stream3 << std::fixed << std::setprecision(1) << temp.V2;
	roundedString = stream3.str();
	ui.V2->setPlainText(QString::fromStdString(roundedString));

	std::ostringstream stream4;
	stream4 << std::fixed << std::setprecision(1) << temp.IPhase;
	roundedString = stream4.str();
	ui.IPhase->setPlainText(QString::fromStdString(roundedString));
}

void Monitoring::Update()
{
	data.push_back(Read());
	//StepThreading();
	if (!work->port->isOpen())Disconnect();
}