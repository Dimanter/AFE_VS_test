#include "StepWindow.hpp"

bool StepWindow::Contains(vector<Data> cont, float _agnle)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].angleGrad == _agnle)
			return true;
	}
	return false;
}

bool StepWindow::Contains(vector<Data> cont, int _angle, int _min)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].min == _min)
			if (cont[i].angle == _angle)
				return true;
	}
	return false;
}

bool StepWindow::Contains(vector<Data> cont, int _angle, int _min, int _sec)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].min == _min)
			if (cont[i].angle == _angle)
				if (cont[i].sec == _sec)
					return true;
	}
	return false;
}

bool StepWindow::Plus(vector<int> cont)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i] < 0)return false;
	}
	return true;
}

bool StepWindow::Minus(vector<int> cont)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i] > 0)
		{
			return false;
		}
	}
	return true;
}

bool StepWindow::CheckDiap(int _angle, int _min)
{
	for (int i = 0; i < 5; i++)
	{
		if (_angle >= Diap[i][0] && _angle < Diap[i][1])return true;
		if (_angle == Diap[i][1] && _min < 1)return true;
	}
	return false;
}

StepWindow::StepWindow(const QString& device, QWidget* parent)
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

	work = new Work(device, 2400, 1400);
	connect(work, &Work::portStatus, this, &StepWindow::StatusConnect);

	Dir = 0;
	Period = 200;
	StepRatio = 32;
	Count = 172800;

	CreateWindow();
	window->showMaximized();
	AplyOpt(Dir, Period, StepRatio, Count);

	timer->setInterval(1.f);
	connect(timer, SIGNAL(timeout()), this, SLOT(Update()));

	testTimer->setInterval(1.f);
	connect(testTimer, SIGNAL(timeout()), this, SLOT(TestUp()));

	qApp->setPalette(darkPalette);
	QApplication::exec();
}

void StepWindow::StepThreading()
{
	ConvertAngle(work->measure->refAngle);
	textAngle->setText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + to_string(sec) + "'" + "'"));
	I->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.RMS)));
	PhaseV2->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.Phase)));
	PhaseV1->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.Phase)));
	RmsV2->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.RMS)));
	RmsV1->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.RMS)));
	IPhase->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.Phase)));
}

void StepWindow::Aply()
{
	Dir = DirBox->currentIndex();
	Period = textPeriod->toPlainText().toInt();
	StepRatio = Steps->currentText().toInt();
	Count = textCount->toPlainText().toInt();
	string temp = Dir == 0 ? "Forward" : "Backward";
	plainTextEdit->setPlainText(QString::fromStdString("Dir - " + temp + "\n") +
		QString::fromStdString("Period - " + to_string(Period) + "\n") +
		QString::fromStdString("Step rate - 1/" + to_string(StepRatio) + "\n") +
		QString::fromStdString("Count steps - " + to_string(Count) + "\n"));
}

void StepWindow::AplyOpt(int _dir, int _period, int _step, int _count)
{
	string temp = _dir == 0 ? "Forward" : "Backward";
	plainTextEdit->setPlainText(QString::fromStdString("Dir - " + temp) + "\n" +
		QString::fromStdString("Period - " + to_string(_period) + "\n") +
		QString::fromStdString("Step rate - 1/" + to_string(_step) + "\n") +
		QString::fromStdString("Count steps - " + to_string(_count) + "\n"));
}

void StepWindow::CreateWindow()
{
	//Main window ================================================================================
	window->setWindowTitle("Meter СКТ-232Б");
	label = new QLabel("Порт ");

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
	RefreshComBox();


	btnRefreshPort = new QPushButton("Обновить");
	QObject::connect(btnRefreshPort, &QPushButton::clicked, this, &StepWindow::RefreshComBox);
	btnRefreshPort->setMinimumWidth(200);
	btnRefreshPort->setMinimumHeight(50);

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
	QObject::connect(btnReport, &QPushButton::clicked, this, &StepWindow::StartUpConverter);
	btnReport->setMinimumHeight(50);
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


	plainTextEdit = new QPlainTextEdit("");
	plainTextEdit->setReadOnly(true);

	btnConnect = new QPushButton("Подключить");
	QObject::connect(btnConnect, &QPushButton::clicked, this, &StepWindow::Connect);
	btnConnect->setMinimumWidth(200);
	btnConnect->setMinimumHeight(50);
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
	QObject::connect(btnDisconnect, &QPushButton::clicked, this, &StepWindow::Disconnect);
	btnDisconnect->setMinimumWidth(200);
	btnDisconnect->setMinimumHeight(50);
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
	QObject::connect(btnMonitor, &QPushButton::clicked, this, &StepWindow::StartMonitor);
	btnMonitor->setMinimumWidth(200);
	btnMonitor->setMinimumHeight(50);
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


	QPushButton* btn = new QPushButton("Aply option");
	QObject::connect(btn, &QPushButton::clicked, this, &StepWindow::Aply);
	btnStart = new QPushButton("Старт");
	btnStart->setStyleSheet("QPushButton{"
		"background-color: #323232;"
		"color: #66cc00;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #66cc00;"
		"font: bold 14px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");
	QObject::connect(btnStart, &QPushButton::clicked, this, &StepWindow::Start);
	btnStart->setMinimumWidth(200);
	btnStart->setMinimumHeight(50);
	//btnStart->setEnabled(false);

	btnStop = new QPushButton("Стоп");
	QObject::connect(btnStop, &QPushButton::clicked, this, &StepWindow::Stop);
	btnStop->setMinimumWidth(200);
	btnStop->setMinimumHeight(50);
	btnStop->setEnabled(false);
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

	btnTest = new QPushButton("Вернуть");
	QObject::connect(btnTest, &QPushButton::clicked, this, &StepWindow::testCommand);
	btnTest->setMinimumWidth(200);

	//Option stepper window ================================================================================
	textDir = new QTextEdit("");
	QLabel* labelD = new QLabel("Направление");
	DirBox = new QComboBox();
	DirBox->addItem("Forward");
	DirBox->addItem("Backward");
	DirBox->setMaximumWidth(200);

	QLabel* labelS = new QLabel("Step ratio");
	Steps = new QComboBox();
	Steps->addItem("1"); Steps->addItem("2"); Steps->addItem("4"); Steps->addItem("8");
	Steps->addItem("16"); Steps->addItem("32"); Steps->addItem("64"); Steps->addItem("128"); Steps->addItem("256");
	Steps->setMaximumWidth(200);

	textPeriod = new QTextEdit("1300");
	QLabel* labelP = new QLabel("Period (number)");

	textCount = new QTextEdit("70000");
	QLabel* labelC = new QLabel("Count (number)");

	//Data output window ================================================================================
	QLabel* label1 = new QLabel("Угол");
	label1->setFont(font);
	textAngle->setReadOnly(true);
	textAngle->zoomIn(30);
	textAngle->setStyleSheet("QTextEdit{"
		"background-color: #323232;"
		"color: white;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: 50px;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	QLabel* label2 = new QLabel("Rms V1 (mV)");
	RmsV1->setReadOnly(true);


	QLabel* label3 = new QLabel("Напряжение (мВ)");
	label3->setFont(font);
	RmsV2->setReadOnly(true);
	RmsV2->setStyleSheet("QTextEdit{"
		"background-color: #323232;"
		"color: white;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: 50px;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");

	QLabel* label4 = new QLabel("Фаза V1");
	PhaseV1->setReadOnly(true);

	QLabel* label5 = new QLabel("Фаза V2");
	PhaseV2->setReadOnly(true);

	QLabel* label6 = new QLabel("I (mA)");
	label6->setFont(font);
	I->setReadOnly(true);
	I->zoomIn(30);
	I->setMaximumHeight(200);

	QLabel* label7 = new QLabel("Фаза I");
	IPhase->setReadOnly(true);

	QVBoxLayout* lay = new QVBoxLayout;
	QTableView* view = new QTableView(window);
	view->setModel(model);
	view->setStyleSheet("QTableView{"
		"background-color: #323232;"
		"gridline-color: #434343;"
		"color: white;"
		"border-style: outset;"
		"border-width: 2px;"
		"border-radius: 10px;"
		"border-color: #CCCCCC;"
		"font: 18px;"
		"min-width: 10em;"
		"padding: 6px;"
		"}"
		"QPushButton::pressed{"
		"background-color:  #666666;"
		"border-style: inset;"
		"}");
	view->setMinimumHeight(400);
	view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

	lay->addWidget(view);
	StatusCon = new QLabel(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));

	QVBoxLayout* PortV = new QVBoxLayout();
	QVBoxLayout* StartV = new QVBoxLayout();
	QVBoxLayout* ConnectV = new QVBoxLayout();
	QVBoxLayout* AngleV = new QVBoxLayout();
	QVBoxLayout* RmsV = new QVBoxLayout();
	QVBoxLayout* IV = new QVBoxLayout();

	QHBoxLayout* PortH = new QHBoxLayout();
	QHBoxLayout* MeasureH = new QHBoxLayout();
	QHBoxLayout* ConnectH = new QHBoxLayout();

	StartV->addWidget(btnMonitor);
	StartV->addWidget(btnStart);
	StartV->addWidget(btnStop);
	ConnectV->addWidget(btnRefreshPort);
	ConnectV->addWidget(btnConnect);
	ConnectV->addWidget(btnDisconnect);
	PortH->addLayout(ConnectV);
	PortH->addLayout(StartV);

	AngleV->addWidget(label1);
	AngleV->addWidget(textAngle);
	RmsV->addWidget(label3);
	RmsV->addWidget(RmsV2);

	MeasureH->addLayout(AngleV);
	MeasureH->addLayout(RmsV);


	PortV->addWidget(label);
	PortV->addWidget(comboPort1);
	PortV->addLayout(PortH);
	PortV->addWidget(btnReport);
	PortV->addStretch(20);
	ConnectH->addLayout(PortV);
	ConnectH->addLayout(MeasureH);

	QHBoxLayout* layoutMainHor = new QHBoxLayout();
	QVBoxLayout* layoutMainVer = new QVBoxLayout();

	layoutMainVer->addLayout(ConnectH);
	layoutMainVer->addLayout(lay);
	layoutMainVer->addWidget(StatusCon);

	window->setLayout(layoutMainVer);
}

void StepWindow::Connect()
{
	if (!work->Connect(comboPort1->currentText()))return;
	connection = true;
	btnConnect->setEnabled(false);
	btnStart->setEnabled(true);
	btnStop->setEnabled(true);
	StatusCon->setText(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));

	work->Stop();
}

void StepWindow::Disconnect()
{
	work->Stop();
	work->Disconnect();
	connection = false;
	btnConnect->setEnabled(true);
	btnStart->setEnabled(false);
	btnStop->setEnabled(false);
	StatusCon->setText(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));
	timer->stop();
}

void StepWindow::Stop()
{
	work->Stop();
}

void StepWindow::Start()
{
	if (work->stage != Work::Stage::Idle)return;
	if (!Dialog())return;
	work->testCommand();
	work->processRun();
	testTimer->start();
	//TestComplete();
	data.clear();
	allData.clear();
	MinV.clear();
}

void StepWindow::testCommand()
{
	work->ReadyStartMonitor(1, 1, 1300, 22000);
	work->processRun();
}

void StepWindow::StartUpConverter()
{
	setlocale(LC_ALL, "Russian");
	fileNum = 0;
	system("Converter.exe");
}

vector<Data> StepWindow::EraseErrors(vector<Data> cont)
{
	vector<Data> temp;
	temp.push_back(cont[0]);
	for (int i = 1; i < cont.size(); i++)
	{
		if (abs(temp[temp.size() - 1].V2 - cont[i].V2) < 60);
		temp.push_back(cont[i]);
	}
	vector<Data> result;
	for (int i = 3; i < temp.size() - 2; i++)
	{
		float val1 = (temp[i - 1].V2 + temp[i - 2].V2) / 2;
		float val2 = (temp[i + 1].V2 + temp[i + 2].V2) / 2;
		float delta = (val1 + val2) / 10;
		if (val1 < 50)delta = 5;
		if (abs(temp[i].V2 - val1) < delta || abs(temp[i].V2 - val2) < delta)
			result.push_back(temp[i]);
	}
	return result;
}

vector<Data> StepWindow::Erase(vector<Data> cont)
{
	vector<Data> result;
	int start = cont[5].angle;
	int end = cont[cont.size() - 5].angle;
	for (int i = 5; i < cont.size() - 5; i++)
	{
		if (cont[i].angle != start && cont[i].angle != end)
			result.push_back(cont[i]);
	}
	return result;
}

vector<Data> StepWindow::Erase(vector<Data> cont, int index)
{
	vector<Data> temp;
	for (int i = 0; i < cont.size(); i++)
	{
		if (i == index)continue;
		temp.push_back(cont[i]);
	}
	return temp;
}

vector<Data> StepWindow::Erase(vector<Data> cont, int left, int right)
{
	vector<Data> temp;
	for (int i = 0; i < cont.size(); i++)
	{
		if (i > left && i < right)continue;
		temp.push_back(cont[i]);
	}
	return temp;
}

vector<Data> StepWindow::AngleOffset(vector<Data> cont, int minInd)
{
	int angle = cont[minInd].angle;
	int min = cont[minInd].min;
	for (int i = 0; i < cont.size(); i++)
	{
		cont[i].angle -= angle;
		cont[i].min -= min;
		if (cont[i].angle < 0)cont[i].angle += 360;
		if (cont[i].min < 0)
		{
			cont[i].min += 60;
			cont[i].angle -= 1;
		}
	}
	return cont;
}

void StepWindow::Serialization(string file, vector<Data> cont)
{
	stringstream ss;
	for (int i = 0; i < cont.size(); i++)
	{
		ss << cont[i].angle << " " << cont[i].min << "'\t\t";// << cont[i].sec << "'" << "'\t";
		//ss << cont[i].angleGrad << "       ";
		//ss << cont[i].V1 << "       \n";
		ss << cont[i].V2 << "\n";
		//ss << cont[i].Phase1 << "       ";
		//ss << cont[i].Phase1 << "       ";
		//ss << cont[i].I << "       ";
		//ss << cont[i].IPhase << 
	}
	std::ofstream _file(file + ".txt", std::ios_base::out | std::ios_base::app);
	if (_file)
	{
		_file << ss.str();
	}
	_file.close();
}

bool StepWindow::Dialog()
{
	bool ok;
	QString text = QInputDialog::getText(this,
		QString::fromUtf8("Ввод"),
		QString::fromUtf8("Введите номер изделя:"),
		QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	NumberDevice = text;
	return ok;
}

int StepWindow::FindMinV(vector<Data> cont)
{
	float min = cont[0].V2;
	int idx = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].V2 < min)
		{
			min = cont[i].V2;
			idx = i;
		}
	}
	return idx;
}

int StepWindow::MinOffset(vector<int> Angle)
{
	int temp = Angle[0];
	for (int i = 0; i < Angle.size(); i++)
	{
		if (Angle[i] < temp)
		{
			temp = Angle[i];
		}
	}
	return temp;
}

int StepWindow::MaxOffset(vector<int> Angle)
{
	int temp = Angle[0];
	for (int i = 0; i < Angle.size(); i++)
	{
		if (Angle[i] > temp)
		{
			temp = Angle[i];
		}
	}
	return temp;
}

int StepWindow::FindMaxV(vector<Data> cont)
{
	float min = cont[0].V2;
	int idx = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].V2 > min)
		{
			min = cont[i].V2;
			idx = i;
		}
	}
	return idx;
}

int StepWindow::FindMinV(vector<Data> cont, int minAngle, int maxAngle)
{
	int i = 0;
	while (true)
	{
		if (cont[i].angle == minAngle)
		{
			break;
		}
		i++;
	}
	float min = cont[i].V2;
	int idx = i;
	for (int j = i; j <= cont.size(); j++)
	{
		if (cont[j].angle > maxAngle)break;
		if (cont[j].angle == maxAngle && cont[j].min > 0)break;
		if (cont[j].V2 < min)
		{
			min = cont[j].V2;
			idx = j;
		}
	}
	return idx;
}

int StepWindow::FindMinAngle(vector<Data> cont)
{
	int minAngle = cont[0].angle;
	int minMin = cont[0].min;
	int minSec = cont[0].sec;
	int idx = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].angle <= minAngle)
			if (cont[0].min <= minMin)
				if (cont[0].sec < minSec)
				{
					minAngle = cont[0].angle;
					minMin = cont[0].min;
					minSec = cont[0].sec;
					idx = i;
				}
	}
	return idx;
}

int* StepWindow::VectorInMass(vector<Data> cont, bool mins)
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

float StepWindow::AverageU(vector<Data> cont)
{
	float result = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		result += cont[i].V2;
	}
	result /= cont.size();
	return result;
}

float* StepWindow::VectorInMassV(vector<Data> cont)
{
	float* V = new float[cont.size()];
	for (int i = 0; i < cont.size(); i++)
	{
		V[i] = cont[i].V2;
	}
	return V;
}

vector<Data> StepWindow::Average(vector<Data> cont)
{
	vector<Data> temp;
	int* Angle = VectorInMass(cont, false);
	int* Min = VectorInMass(cont, true);
	float* V2 = VectorInMassV(cont);

	for (int i = 0; i < cont.size(); i++)
	{
		int currentMin = Min[i];
		int currentAngle = Angle[i];
		if (Contains(temp, currentAngle, currentMin))continue;
		float sum = 0;
		int k = 0;
		for (int j = i; j < cont.size(); j++)
		{
			if (currentAngle == Angle[j] && currentMin == Min[j])
			{
				sum += V2[j];
				k++;
			}
		}
		sum /= k;

		Data dt;
		dt.angle = currentAngle;
		dt.min = currentMin;
		dt.V2 = sum;
		temp.push_back(dt);
	}
	return temp;
}

void StepWindow::SwapData(Data& val1, Data& val2)
{
	Data tmp;
	tmp.angle = val1.angle;
	tmp.I = val1.I;
	tmp.IPhase = val1.IPhase;
	tmp.min = val1.min;
	tmp.sec = val1.sec;
	tmp.Phase1 = val1.Phase1;
	tmp.Phase2 = val1.Phase2;
	tmp.V1 = val1.V1;
	tmp.V2 = val1.V2;

	val1.angle = val2.angle;
	val1.I = val2.I;
	val1.IPhase = val2.IPhase;
	val1.min = val2.min;
	val1.sec = val2.sec;
	val1.Phase1 = val2.Phase1;
	val1.Phase2 = val2.Phase2;
	val1.V1 = val2.V1;
	val1.V2 = val2.V2;

	val2.angle = tmp.angle;
	val2.I = tmp.I;
	val2.IPhase = tmp.IPhase;
	val2.min = tmp.min;
	val2.sec = tmp.sec;
	val2.Phase1 = tmp.Phase1;
	val2.Phase2 = tmp.Phase2;
	val2.V1 = tmp.V1;
	val2.V2 = tmp.V2;
}

void StepWindow::ConvertAngle(float _angle)
{
	float grad, _min, _sec;
	grad = std::trunc(_angle);
	_min = _angle - grad;
	_min = std::round(_min * 60.0);
	_sec = (_angle - grad) * 60.0 - _min;
	_sec = std::trunc(_sec * 60.0);
	angle = grad;
	min = _min;
	sec = _sec + 30;
}

void StepWindow::TestComplete()
{
	Stop();
	testTimer->stop();
	stringstream ss;//поток для записи данных в файл с минимумами
	stringstream writer;//поток для записи в файл с данными результатов измерений
	//*Запись данных в файл
	Serialization("Data0", data);
	//*
	bool flag = false;//Флаг смены направления(возрастание/убывание)
	bool flagResist = false;

	float Uav = 0;//Среднее напряжение на участке силы тока

	int UavSize = 0;//Размер буфера среднего напряжения
	int k = 0;
	int IndT[13] = { 0, 1, 6, 7, 9, 10, 11, 12, 3, 5, 2, 8, 4 };//Индекс заполнения минимумов

	vector <Data> temp;//Буфер данных
	vector <Data> MaxU;//Контенер для поиска максимума нарпяжения
	vector <Data> Quad;//Контенер для квадратурного напряжения
	// ReadFromFile("Data0");
	vector<Data> adt = data;//Контенер данных с устройства
	vector <int> Grad;//Контенер минимумов напряжения
	vector < string> tmpInd;//Буфер минимумов напряжения

	string tmp = to_string(0) + "'";//Строка вывода минимумов напряжения в таблицу
	tmpInd.push_back(tmp);
	string IndPunkts[13] = { "а","б","в","г","д","е","ж","з","и","л","к1","к2","к3" };

	//*Заполнение пунктов комутации 0,1, 6, 7,9,10,11,12, 3,4,2,8,5
	Punkts["а"] = 0;
	Punkts["б"] = 1;
	Punkts["в"] = 6;
	Punkts["г"] = 7;
	Punkts["д"] = 9;
	Punkts["е"] = 10;
	Punkts["ж"] = 11;
	Punkts["з"] = 12;
	Punkts["и"] = 3;
	Punkts["л"] = 5;
	Punkts["к1"] = 2;
	Punkts["к2"] = 8;
	Punkts["к3"] = 4;
	//*
	//*Убираем ошибочные данные
	//adt = EraseErrors(adt);
	temp.push_back(adt[0]);
	temp.push_back(adt[1]);

	//*Разбиваем на фрагменты комутации
	try
	{
		for (int i = 2; i < adt.size(); i++)
		{
			if (flagResist)
			{
				if (adt[i].V2 < 500)
					Quad.push_back(adt[i]);
				continue;
			}
			//*Участок поиска силы тока
			if (adt[i].angle > 20 && adt[i].angle < 45)
			{
				if (adt[i].V2 < 100)
				{
					Uav += adt[i].V2;
					UavSize++;
				}
			}
			//*Участок поиска Umax
			if (adt[i].angle >= 110 && adt[i].angle < 180 && !Contains(MaxU, adt[i].angle))
			{
				MaxU.push_back(adt[i]);
			}
			//*Участок поиска квадратурного напряжения
			if ((adt[i].angle >= 201 && adt[i].angle < 269) || adt[i].angle >= 291)
			{
				if (adt[i].V2 < 500)
					Quad.push_back(adt[i]);
				if (adt[i].angle > 350)flagResist = true;
			}
			//*Смена при перепрыжке с неизмеремого участка на измеряемый
			if (!CheckDiap(adt[i].angle, adt[i].min))
			{
				if (temp.size() > 100)
				{
					allData.push_back(temp);
					temp.clear();
				}
				continue;
			}
			//*Смена с Возрастающего на убывающий
			if (adt[i + 1].angle == adt[i].angle && adt[i + 1].min < adt[i].min && !flag && adt[i + 2].min < adt[i].min)
			{
				flag = true;
				if (temp.size() > 100)
				{
					allData.push_back(temp);
					temp.clear();
				}
			}
			//*Смена с убывающего на возрастабщий
			if (adt[i + 1].angle == adt[i].angle && adt[i + 1].min > adt[i].min && flag && adt[i + 2].min > adt[i].min)
			{
				flag = false;
			}
			if (!flag)
				temp.push_back(adt[i]);
		}
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить разделение данных");
		return;
	}
	//*
	if (allData.size() < 12)
	{
		QMessageBox::warning(window, "Error", "Недостаточно данных");
		//return;
	}
	//*Убираем ошибки при переключении схемы комутации и усредняем данные
	Quad = EraseErrors(Quad);
	Serialization("Quad", Quad);
	int maxU = FindMaxV(MaxU);//Индекс максимального напряжения
	Uav /= UavSize;
	float Isr = Uav / 10;//Сила тока
	try
	{
		for (int i = 0; i < allData.size(); i++)
		{
			allData[i] = EraseErrors(allData[i]);
			allData[i] = Average(allData[i]);
			allData[i] = Erase(allData[i]);
			allData[i] = Erase(allData[i]);
			MinV.push_back(FindMinV(allData[i]));
			if (allData[i][MinV[i]].V2 > 35.f)
			{
				QMessageBox::warning(window, "Error", "Umin > 30 \n" +
					QString::fromStdString(to_string(allData[i][MinV[i]].angle) + " " + to_string(allData[i][MinV[i]].min) + "'"
						+ " " + to_string(allData[i][MinV[i]].V2)));
				//return;
			}
			Serialization("av" + to_string(i), allData[i]);
		}
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных");
		return;
	}

	//*
	Grad.push_back(0);
	//*Поиск минимумов на всех схемах комутации
	ss << "№ \t Диапазон угла \tУгловое отклонение роторов \n";
	try
	{
		for (int i = 1; i < allData.size(); i++)
		{
			int offs = (i / (3 - i / 11)) - i / 11 - (i / 12);
			int angleT = allData[i][MinV[i]].angle - Diap[offs][0];
			int minT = allData[i][MinV[i]].min;

			int angleM = allData[0][MinV[0]].angle * 60 + allData[0][MinV[0]].min;
			int delta = (angleT * 60 + minT) - angleM;
			angleT = delta / 60;
			minT = delta % 60;


			ss << offs << " - " << Diap[offs][0] << " = ";
			ss << angleT << " " << minT << "'\n";
			/*if(angleT!=0)
				tmp = to_string(angleT) + " " + to_string(minT) + "'";
			else*/
			tmp = to_string(minT) + "'";
			Grad.push_back(minT);
			tmpInd.push_back(tmp);
		}
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить отклонение роторов");
		return;
	}
	//*
	vector<int> minmax;
	float neravRotor;
	float neravStator;
	float neravU;
	float elAssimetry;

	//*Заполнение таблицы
	try
	{
		neravRotor = abs(Grad[Punkts["и"]] - Grad[Punkts["в"]] / 2.f);//Нера-во коэф. Ротора
		neravStator = abs(Grad[Punkts["к3"]] - (Grad[Punkts["к2"]] + Grad[Punkts["к1"]]) / 2.f);//Нера-во коэф. Статора
		neravU = abs(Grad[Punkts["л"]] - (Grad[Punkts["г"]] / 2.f));//Нера-во сопротивления
		elAssimetry = (abs(MinOffset(Grad)) + abs(MaxOffset(Grad))) / 2.f;//Электромаг. Ассиметрия
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить коэфиценты");
		return;
	}

	model->setHeaderData(0, Qt::Horizontal, QString::fromStdString("I (mA)"));//Usr / 10
	model->setHeaderData(1, Qt::Horizontal, QString::fromStdString("Umax(V)"));//110 - 180grad max
	try
	{
		for (auto t : Punkts)
		{
			if (t.first <= "з" && t.first >= "б")
				minmax.push_back(Grad[Punkts[t.first]]);
			model->setHeaderData(k + 2, Qt::Horizontal, QString::fromStdString(IndPunkts[k]));
			model->setItem(loop, k + 2, new QStandardItem(QString::fromStdString(tmpInd[IndT[k]])));
			writer << tmpInd[IndT[k]] << ";";
			model->setItem(0, k + 2, new QStandardItem(QString::fromStdString(to_string(Diap[Ind[k]][0]))));
			k++;
		}
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить коэфиценты");
		return;
	}

	try
	{
		writer << "\n";
		k = Punkts.size() + 2;
		model->setItem(0, 0, new QStandardItem(QString::fromStdString("<15")));
		model->setItem(0, 1, new QStandardItem(QString::fromStdString(">7")));
		if (Plus(minmax) && Minus(minmax))
			elAssimetry = (abs(MinOffset(minmax)) + abs(MaxOffset(minmax))) / 2.f;
		else
			elAssimetry = Plus(minmax) ? abs(MaxOffset(minmax)) / 2.f : abs(MinOffset(minmax)) / 2.f;

		Isr *= 2;
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить ассиметрию");
		return;
	}

	//*Цветные ячейки
	int QuadM = FindMaxV(Quad);
	QStandardItem* IsrText = new QStandardItem(QString::fromStdString(to_string(Isr).erase(4, 8)));
	QStandardItem* UText = new QStandardItem(QString::fromStdString(to_string(neravU).erase(4, 8)));
	QStandardItem* elText = new QStandardItem(QString::fromStdString(to_string(elAssimetry).erase(4, 8)));
	QStandardItem* RotorText = new QStandardItem(QString::fromStdString(to_string(neravRotor).erase(4, 8)));
	QStandardItem* StatorText = new QStandardItem(QString::fromStdString(to_string(neravStator).erase(4, 8)));
	QStandardItem* QuadText = new QStandardItem(QString::fromStdString(to_string((Quad[QuadM].V2)).erase(5, 9)));
	QStandardItem* UMaxText = new QStandardItem(QString::fromStdString(to_string((MaxU[maxU].V2 / 1000)).erase(4, 8)));

	writer << Isr << "\n" << MaxU[maxU].V2 / 1000 << "\n" << elAssimetry << "\n" << neravRotor << "\n"
		<< neravStator << "\n" << neravU << "\n" << Quad[QuadM].V2 << "\n";

	Isr > 15 ? IsrText->setBackground(red) : IsrText->setBackground(green);
	(MaxU[maxU].V2 / 1000) < 7 ? UMaxText->setBackground(red) : UMaxText->setBackground(green);
	neravU <= 10 ? UText->setBackground(green) : (neravU <= 20 ? UText->setBackground(yellow) : UText->setBackground(red));
	elAssimetry <= 10 ? elText->setBackground(green) : (elAssimetry <= 20 ? elText->setBackground(yellow) : elText->setBackground(red));
	neravRotor <= 10 ? RotorText->setBackground(green) : (neravRotor <= 20 ? RotorText->setBackground(yellow) : RotorText->setBackground(red));
	neravStator <= 10 ? StatorText->setBackground(green) : (neravStator <= 20 ? StatorText->setBackground(yellow) : StatorText->setBackground(red));
	Quad[QuadM].V2 <= 150 ? QuadText->setBackground(green) : (Quad[QuadM].V2 <= 200 ? QuadText->setBackground(yellow) : QuadText->setBackground(red));

	//*Заполнения таблицы 
	model->setItem(loop, 0, IsrText);
	model->setItem(loop, 1, UMaxText);
	model->setItem(loop, 2, new QStandardItem("0"));
	model->setHeaderData(k, Qt::Horizontal, "Электромаг.\n Ассиметрия");
	model->setItem(0, k, new QStandardItem("<10|<20"));
	model->setItem(loop, k, elText);
	k++;
	model->setHeaderData(k, Qt::Horizontal, "Нера-во коэф.\n Ротора");
	model->setItem(0, k, new QStandardItem("<10|<20"));
	model->setItem(loop, k, RotorText);
	k++;
	model->setHeaderData(k, Qt::Horizontal, "Нера-во коэф.\n Статора");
	model->setItem(0, k, new QStandardItem("<10|<20"));
	model->setItem(loop, k, StatorText);
	k++;
	model->setHeaderData(k, Qt::Horizontal, "Нера-во\nсопротивления");
	model->setItem(0, k, new QStandardItem("<10|<20"));
	model->setItem(loop, k, UText);
	k++;
	model->setHeaderData(k, Qt::Horizontal, "Квадратур.\nнапряжение(мВ)");
	model->setItem(0, k, new QStandardItem("<150|<200"));
	model->setItem(loop, k, QuadText);
	k++;
	model->setHeaderData(k, Qt::Horizontal, "Фаза");
	model->setItem(0, k, new QStandardItem("СТУ"));
	model->setItem(loop, k, new QStandardItem("СТУ"));
	k++;
	model->setHeaderData(k, Qt::Horizontal, "Номер изделия");
	model->setItem(0, k, new QStandardItem(""));
	model->setItem(loop, k, new QStandardItem(NumberDevice));
	k++;

	/*ss << "Нерав-во коэф. (ротор) = " << neravRotor << "\n";
	ss << "Нерав-во коэф. (статор) = " << neravStator << "\n";
	ss << "Нерав-во сопротивления = " << neravU << "\n";
	ss << "Электромагнитная ассиметрия = " << elAssimetry << "\n";

	string file = "min";
	remove("min.txt");
	std::ofstream _file(file + ".txt", std::ios_base::out | std::ios_base::app);
	if (_file)
	{
		_file << ss.str();
	}
	_file.close();*/
	//запись в результатов файл 
	writer << NumberDevice.toStdString();
	string file1 = "test" + to_string(fileNum);
	std::ofstream _file1(file1 + ".txt", std::ios_base::out);
	if (_file1)
	{
		_file1 << writer.str();
	}
	_file1.close();
	loop++;
	fileNum++;
}

void StepWindow::StartMonitor()
{
	if (!work->port->isOpen())return;
	timer->start();
	work->Monitor();
}

void StepWindow::StatusConnect(bool connected)
{
	btnConnect->setEnabled(!connected);
}

void StepWindow::RefreshComBox()
{
	comboPort1->clear();
	const auto serialPortInfos = QSerialPortInfo::availablePorts();
	for (const QSerialPortInfo& portInfo : serialPortInfos)
	{
		comboPort1->insertItem(portInfo.productIdentifier(), portInfo.portName());
	}
}

void StepWindow::TestUp()
{
	if (work->ProccessStatus())TestComplete();

	float ref = work->measure->refAngle;
	ConvertAngle(ref);
	if (Contains(data, ref))return;
	data.push_back(Read());
}

void StepWindow::Update()
{
	StepThreading();
	if (!work->port->isOpen())Disconnect();
}

vector<Data> StepWindow::ReadFromFile(string file)
{
	vector <Data> temp;
	string text;
	string all = "";
	std::ifstream _file(file + ".txt", std::ios_base::out | std::ios_base::app);
	if (_file.is_open())
	{
		while (std::getline(_file, text))
		{
			float numbers[4] = { 0 };
			Data tmp;
			string num = "";
			int k = 0;
			for (int i = 0; i < text.size(); i++)
			{
				if (text[i] == ' ' || text[i] == '\'' && num.size() != 0)
				{
					numbers[k] = atoi(num.c_str());
					//plainTextEdit->insertPlainText(QString::fromStdString(to_string(numbers[k]) + " "));
					num = "";
					k++;
					continue;
				}
				num += text[i];
			}
			bool dec = false;
			string flo = "";
			for (int i = 0; i < num.size(); i++)
			{
				if (num[i] == ' ')continue;
				if (num[i] == '.')
				{
					numbers[k] = atof(flo.c_str());
					//plainTextEdit->insertPlainText(QString::fromStdString(to_string(numbers[k])) + ".");
					dec = true;
					flo = "";
					continue;
				}
				flo += num[i];
			}
			if (dec)
				numbers[k] += atof(flo.c_str()) / pow(10, flo.size());
			else
				numbers[k] = atof(flo.c_str());
			//plainTextEdit->insertPlainText(QString::fromStdString(to_string(numbers[k])) + "\n");
			tmp.angle = numbers[0];
			tmp.min = numbers[1];
			tmp.V2 = numbers[k];
			temp.push_back(tmp);
		}
	}
	_file.close();
	return temp;
}

Data StepWindow::Read()
{
	Data temp;

	temp.angle = angle;
	temp.min = min;
	temp.sec = sec;
	temp.angleGrad = work->measure->refAngle;
	//temp.V1 = measures_t(work->measure->Result).V1.RMS;
	temp.V2 = measures_t(work->measure->Result).V2.RMS;
	//temp.Phase1 = measures_t(work->measure->Result).V1.Phase;
	//temp.Phase2 = measures_t(work->measure->Result).V2.Phase;
	temp.I = measures_t(work->measure->Result).I.RMS;
	//temp.IPhase = measures_t(work->measure->Result).I.Phase;

	return temp;
}

StepWindow::~StepWindow()
{
	timer->stop();
	testTimer->stop();
	if (!work->port->isOpen())return;
	work->Stop();
	work->disconnect();
	free(work);
}
