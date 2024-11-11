#include "StepWindow.hpp"

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
	DeviceName = device;

	Dir = 0;
	Period = 200;
	StepRatio = 32;
	Count = 172800;

	if (device == "45Д20-2")
	{
		CreateWindow45D20();
		window->showMaximized();	
	}
	else if (device == "СКТ-232Б")
	{
		CreateWindowSKT();
		window->showMaximized();
	}
	else if (device == "СКТ-265Д")
	{

	}

	timer->setInterval(1.f);
	connect(timer, SIGNAL(timeout()), this, SLOT(Update()));

	testTimer->setInterval(1.f);
	connect(testTimer, SIGNAL(timeout()), this, SLOT(Test45D20()));

	testSktTimer->setInterval(1.f);
	connect(testSktTimer, SIGNAL(timeout()), this, SLOT(TestSKT232B()));
	
	CreateTable();
	qApp->setPalette(darkPalette);
	QApplication::exec();
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
	_min = std::trunc(_min * 60.0);
	_sec = (_angle - grad) * 60.0 - _min;
	_sec = std::trunc(_sec * 60.0);
	angle = grad;
	min = _min;
	sec = _sec;
}

void StepWindow::TestComplete()
{
	Stop();
	stringstream writer;
	testSktTimer->stop();
	//*Запись данных в файл
	Serialization("Data0", data);
	//*
	//data = ReadFromFile("Data0");
	bool flag = false;//Флаг смены направления(возрастание/убывание)
	bool flagResist = false;

	float Uav = 0;//Среднее напряжение на участке силы тока

	int UavSize = 0;//Размер буфера среднего напряжения
	int k = 0;
	int IndT[13] = { 0, 1, 6, 7, 9, 10, 11, 12, 3, 5, 2, 8, 4 };//Индексы заполнения минимумов

	vector <Data> temp;//Буфер данных
	vector <Data> MaxU;//Контенер для поиска максимума нарпяжения
	vector <Data> Quad;//Контенер для квадратурного напряжения
	
	vector<Data> adt = data;//Контенер данных с устройства
	vector <int> Grad;//Контенер минимумов напряжения
	vector < string> tmpInd;//Буфер минимумов напряжения
	string tmp = to_string(0) + "'";//Строка вывода минимумов напряжения в таблицу
	tmpInd.push_back(tmp);
	string IndPunkts[13] = { "а","б","в","г","д","е","ж","з","и","л","к1","к2","к3" };
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
			//*Участок поиска квадратурного напряжения
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
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить разделение данных, проверьте правильность установки датчика.");
		return;
	}
	//*
	if (allData.size() < 12 || allData.size() > 13)
	{
		QMessageBox::warning(window, "Error", "Проверьте правильность подключения");
		return;
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
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}

	//*
	Grad.push_back(0);
	//*Поиск минимумов на всех схемах комутации
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

			tmp = to_string(minT) + "'";
			Grad.push_back(minT);
			tmpInd.push_back(tmp);
		}
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить отклонение роторов, проверьте правильность установки датчика.");
		return;
	}
	//*
	vector<int> minmax;
	float neravRotor;//неравенство ротора
	float neravStator;//неравенство статора
	float neravU;//неравенство напряжения
	float elAssimetry;//электронная ассиметрия 

	try
	{
		neravRotor = abs(Grad[Punkts["и"]] - Grad[Punkts["в"]] / 2.f);//Нера-во коэф. Ротора
		neravStator = abs(Grad[Punkts["к3"]] - (Grad[Punkts["к2"]] + Grad[Punkts["к1"]]) / 2.f);//Нера-во коэф. Статора
		neravU = abs(Grad[Punkts["л"]] - (Grad[Punkts["г"]] / 2.f));//Нера-во сопротивления
		//elAssimetry = (abs(MinOffset(Grad)) + abs(MaxOffset(Grad))) / 2.f;//Электромаг. Ассиметрия
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить коэфиценты, проверьте правильность установки датчика.");
		return;
	}

	try
	{
		for (auto t : Punkts)
		{
			writer << tmpInd[IndT[k]] << ";";
			model->setItem(loop, k + 2, new QStandardItem(QString::fromStdString(tmpInd[IndT[k]])));
			k++;
		}
		minmax.push_back(Grad[Punkts["б"]]);
		minmax.push_back(Grad[Punkts["в"]]);
		minmax.push_back(Grad[Punkts["г"]]);
		minmax.push_back(Grad[Punkts["д"]]);
		minmax.push_back(Grad[Punkts["е"]]);
		minmax.push_back(Grad[Punkts["ж"]]);
		minmax.push_back(Grad[Punkts["з"]]);
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить коэфиценты, проверьте правильность установки датчика.");
		return;
	}


	try
	{
		k = Punkts.size() + 2;
		if (Plus(minmax) && Minus(minmax))
			elAssimetry = (abs(MinOffset(minmax)) + abs(MaxOffset(minmax))) / 2.f;
		else
			elAssimetry = Plus(minmax) ? abs(MaxOffset(minmax)) / 2.f : abs(MinOffset(minmax)) / 2.f;
		Isr *= 2;
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить ассиметрию, проверьте правильность установки датчика.");
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

	writer << "\n" << Isr << "\n" << MaxU[maxU].V2 / 1000 << "\n" << elAssimetry 
		<< "\n" << neravRotor << "\n"<< neravStator << "\n" << neravU << "\n" << Quad[QuadM].V2 << "\n";

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
	model->setItem(loop, 2, new QStandardItem("0'"));
	k = 15;
	model->setItem(loop, k, elText);
	k++;
	model->setItem(loop, k, RotorText);
	k++;
	model->setItem(loop, k, StatorText);
	k++;
	model->setItem(loop, k, UText);
	k++;
	model->setItem(loop, k, QuadText);
	k++;
	model->setItem(loop, k, new QStandardItem("СТУ"));
	k++;
	model->setItem(loop, k, new QStandardItem(NumberDevice));
	k++;

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

void StepWindow::Test45D20Complete()
{
	work->Stop();
	stringstream writer;//потоковая запись результатов измерений в файл
	vector<float> AngleRes;//расчётное напряжения на углах расчёта погрешности
	vector<Data> ErrRate;//измеренные данные на углах расчёта погрешности
	float Isr;//потребляемый ток
	float angleEx;//выходное напряжение на угле -40 градусов
	float U40;//выходное напряжение  на угле 40 градусов
	float Ucomp;//компенсационное напряжения
	float Ucm;//коэфицент классности
	float delta;//сдвиг напряжения при изменении угла в 5 градусов
	int clas;//класс прибора
	int k = 0;

	Serialization("data1", data);
	data = EraseErrors(data);

	int tempMin = FindMinV(data, 40, 50);//индекс элемента с минимальным напряжением 
	float tempMinData = data[tempMin].V1;//минимум напряжения 
	try
	{
		data = Average(data);//усреднение данных
		data = OffsetToZero(data);//сдвиг данных относительно минимального элемента
		Serialization("offsetData", data);
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}
	try
	{
		Isr = AverageI(data);
		angleEx = data[FindAngle(-40, 0, data)].V1;
		U40 = data[FindAngle(40, 0, data)].V1;
		Ucomp = data[FindAngle(40, 0, data)].V2;
		Ucm = Ucomp / U40;
		delta = angleEx / 8;
		clas = 4;
		if (Ucm >= 0.0481 && Ucm <= 0.0485)clas = 1;
		else if (Ucm >= 0.0479 && Ucm <= 0.0487)clas = 2;
		else if (Ucm >= 0.0474 && Ucm <= 0.0491)clas = 3;
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}

	AngleRes.push_back(angleEx);
	ErrRate.push_back(FindNearestAngle(angleEx, -40, data));
	k++;

	int angleRef = -35;
	try
	{
		for (int i = 0; i < 16; i++)
		{
			if (i == 7)continue;
			AngleRes.push_back(abs(angleEx - delta * (i + 1)));
			ErrRate.push_back(FindNearestAngle(AngleRes[k], angleRef, data));
			angleRef += 5;
			if (angleRef == 0)angleRef += 5;
			k++;
		}
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}


	vector<int> ContMin;
	try
	{

		angleRef = 40;
		k = 2;
		for (int i = 0; i < ErrRate.size(); i++)
		{
			int temp = abs(ErrRate[i].angle) - abs(angleRef);
			int MinR = temp * 60 + ErrRate[i].min;
			ContMin.push_back(MinR);
			model->setItem(loop, k, new QStandardItem(QString::fromStdString(to_string(MinR) + "'")));
			writer << MinR << "';";
			angleRef -= 5;
			if (angleRef == 0)angleRef -= 5;
			k++;
		}
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}

	float Uex = FindUex(ContMin);//максимальная погрешность

	writer << "\n" << Isr << "\n" << tempMinData << "\n" << Uex 
		<< "\n" << Ucm << "\n" << U40 << "\n" << clas << "\n" << NumberDevice.toStdString();
	//цветные ячейки
	QStandardItem* MinUText = new QStandardItem(QString::fromStdString(to_string(tempMinData)));
	QStandardItem* UexText = new QStandardItem(QString::fromStdString(to_string(Uex)));
	QStandardItem* UcmText = new QStandardItem(QString::fromStdString(to_string(Ucm)));
	QStandardItem* ClasText = new QStandardItem(QString::fromStdString(to_string(clas)));
	QStandardItem* U40Text = new QStandardItem(QString::fromStdString(to_string(U40)));
	QStandardItem* IsrText = new QStandardItem(QString::fromStdString(to_string(Isr)));

	tempMinData > 2.5 ? MinUText->setBackground(red) : MinUText->setBackground(green);
	abs(Uex) > 0.7 ? UexText->setBackground(red) : UexText->setBackground(green);
	Isr > 6.0 ? IsrText->setBackground(red) : IsrText->setBackground(green);
	if (clas == 1)
	{
		UcmText->setBackground(green);
		ClasText->setBackground(green);
	}
	if (clas == 2)
	{
		UcmText->setBackground(yellow);
		ClasText->setBackground(yellow);
	}
	if (clas == 3)
	{
		UcmText->setBackground(orange);
		ClasText->setBackground(orange);
	}
	if (clas == 4)
	{
		UcmText->setBackground(red);
		ClasText->setBackground(red);
	}
	U40 > 3300 ? U40Text->setBackground(red) : U40 < 2700 ? U40Text->setBackground(red) : U40Text->setBackground(green);
	//вывод данных в таблицу
	model->setItem(loop, 0, IsrText);
	model->setItem(loop, 1, MinUText);
	model->setItem(loop, 18, UexText);
	model->setItem(loop, 19, UcmText);
	model->setItem(loop, 20, ClasText);
	model->setItem(loop, 21, U40Text);
	model->setItem(loop, 22, new QStandardItem(NumberDevice));
	//запись результатов измерения в файл
	std::ofstream _file("test" + to_string(fileNum) + ".txt", std::ios_base::out);
	if (_file)
	{
		_file << writer.str();
	}
	_file.close();
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

void StepWindow::TestSKT232B()
{
	if (work->ProccessStatus()) TestComplete();
	float ref = work->measure->refAngle;
	ConvertAngle(ref);
	if (Contains(data, ref))return;
	data.push_back(Read());
}

void StepWindow::Test45D20()
{
	if (work->ProccessStatus())Test45D20Complete();
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

void StepWindow::StepThreading()
{
	ConvertAngle(work->measure->refAngle);
	textAngle->setText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + to_string(sec) + "'" + "'"));
	I->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.RMS)));
	PhaseV2->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.Phase)));
	PhaseV1->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.Phase)));
	RmsV1->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.RMS)));
	if (DeviceName == "45Д20-2")
	{
		RmsV2->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.RMS)));
	}
	else
	{
		RmsV2->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.RMS)));
	}
	IPhase->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.Phase)));
}

void StepWindow::CreateWindowSKT()
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
	QPushButton* btnDelete = new QPushButton("Удалить строку");
	QObject::connect(btnDelete, &QPushButton::clicked, this, &StepWindow::DeleteRow);
	btnDelete->setMinimumHeight(50);
	btnDelete->setStyleSheet("QPushButton{"
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

	btnMonitor = new QPushButton("Монитор");
	QObject::connect(btnMonitor, &QPushButton::clicked, this, &StepWindow::StartMonitor);
	btnMonitor->setMinimumWidth(200);
	btnMonitor->setMinimumHeight(50);
	btnMonitor->setStyleSheet("QPushButton{"
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

	btnStart = new QPushButton("Старт");
	btnStart->setStyleSheet("QPushButton{"
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
	QObject::connect(btnStart, &QPushButton::clicked, this, &StepWindow::StartSKT);
	btnStart->setMinimumWidth(200);
	btnStart->setMinimumHeight(50);
	btnStart->setEnabled(false);

	btnStop = new QPushButton("Стоп");
	QObject::connect(btnStop, &QPushButton::clicked, this, &StepWindow::Stop);
	btnStop->setMinimumWidth(200);
	btnStop->setMinimumHeight(50);
	btnStop->setEnabled(false);
	btnStop->setStyleSheet("QPushButton{"
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

	QToolBar* toolBar = new QToolBar("control tool bar");
	QPushButton* aboutButton = new QPushButton("О программе");
	QObject::connect(aboutButton, &QPushButton::clicked, this, &StepWindow::AboutProgramm);
	toolBar->addWidget(aboutButton);

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
	PortV->addWidget(btnDelete);
	PortV->addStretch(20);
	
	ConnectH->addLayout(PortV);
	ConnectH->addLayout(MeasureH);

	QHBoxLayout* layoutMainHor = new QHBoxLayout();
	QVBoxLayout* layoutMainVer = new QVBoxLayout();

	layoutMainVer->addWidget(toolBar);
	layoutMainVer->addLayout(ConnectH);
	layoutMainVer->addLayout(lay);
	layoutMainVer->addWidget(StatusCon);

	window->setLayout(layoutMainVer);
}

void StepWindow::CreateWindow45D20()
{
	//Main window ================================================================================
	window->setWindowTitle("Meter 45Д20-2");
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
	QPushButton* btnDelete = new QPushButton("Удалить строку");
	QObject::connect(btnDelete, &QPushButton::clicked, this, &StepWindow::DeleteRow);
	btnDelete->setMinimumHeight(50);
	btnDelete->setStyleSheet("QPushButton{"
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

	btnMonitor = new QPushButton("Монитор");
	QObject::connect(btnMonitor, &QPushButton::clicked, this, &StepWindow::StartMonitor);
	btnMonitor->setMinimumWidth(200);
	btnMonitor->setMinimumHeight(50);
	btnMonitor->setStyleSheet("QPushButton{"
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

	btnStart = new QPushButton("Старт");
	btnStart->setStyleSheet("QPushButton{"
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
	QObject::connect(btnStart, &QPushButton::clicked, this, &StepWindow::Start45D20);
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

	QToolBar* toolBar = new QToolBar("control tool bar");
	QPushButton* aboutButton = new QPushButton("О программе");
	QObject::connect(aboutButton, &QPushButton::clicked, this, &StepWindow::AboutProgramm);
	toolBar->addWidget(aboutButton);

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
	PortV->addWidget(btnDelete);
	PortV->addStretch(20);

	ConnectH->addLayout(PortV);
	ConnectH->addLayout(MeasureH);

	QHBoxLayout* layoutMainHor = new QHBoxLayout();
	QVBoxLayout* layoutMainVer = new QVBoxLayout();

	layoutMainVer->addWidget(toolBar);
	layoutMainVer->addLayout(ConnectH);
	layoutMainVer->addLayout(lay);
	layoutMainVer->addWidget(StatusCon);

	window->setLayout(layoutMainVer);
}

void StepWindow::ActivateButton()
{
	btnConnect->setEnabled(false);
	btnStart->setEnabled(true);
	btnStop->setEnabled(true);
	btnConnect->setStyleSheet("QPushButton{"
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
}

void StepWindow::DiactivateButton()
{
	btnConnect->setEnabled(true);
	btnStart->setEnabled(false);
	btnStop->setEnabled(false);

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

	btnStart->setStyleSheet("QPushButton{"
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

	btnMonitor->setStyleSheet("QPushButton{"
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

	btnStop->setStyleSheet("QPushButton{"
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
}

void StepWindow::Connect()
{
	work->Connect(comboPort1->currentText());
	work->Stop();
	connection = true;
	StatusCon->setText(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));
	ActivateButton();
}

void StepWindow::Disconnect()
{
	work->Stop();
	work->Disconnect();
	connection = false;
	DiactivateButton();
	StatusCon->setText(QString::fromStdString(connection == true ? "Статус : подключено" : "Статус : отключено"));
	timer->stop();
}

void StepWindow::Stop()
{
	work->Stop();
}

void StepWindow::StartSKT()
{
	if (fileNum > 48)
	{
		QMessageBox::warning(window, "Error", "Документ переполнен. Создайте отчёт.");
		return;
	}
	if (work->stage != Work::Stage::Idle)return;
	if (!Dialog())return;
	work->testSKT();
	work->processRun();
	

	testSktTimer->start();
	data.clear();
	allData.clear();
	MinV.clear();
	//TestComplete();
}

void StepWindow::Start45D20()
{
	if (!Dialog())return;
	work->test45D20();
	work->processRun();
	testTimer->start();
	data.clear();
}

void StepWindow::StartUpConverter()
{
	if (fileNum != 0 && DeviceName == "СКТ-232Б")
	{
		system("del test*.txt");
		string t;
		for (int i = 1; i < fileNum + 1; i++)
		{
			stringstream writer;
			writer << model->item(i, 2)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 3)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 4)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 5)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 6)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 7)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 8)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 9)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 10)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 11)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 12)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 13)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 14)->data(Qt::DisplayRole).toString().toStdString() + ";\n";
			writer << model->item(i, 0)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 1)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 15)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 16)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 17)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 18)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 19)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 21)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			string file1 = "test" + to_string(i - 1);
			std::ofstream _file1(file1 + ".txt", std::ios_base::out);
			if (_file1)
			{
				_file1 << writer.str();
			}
			_file1.close();
		}
	}
	if (fileNum != 0 && DeviceName == "45Д-20-2")
	{
		system("del test*.txt");
		string t;
		for (int i = 1; i < fileNum + 1; i++)
		{
			stringstream writer;
			writer << model->item(i, 2)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 3)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 4)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 5)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 6)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 7)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 8)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 9)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 10)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 11)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 12)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 13)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 14)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 15)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 16)->data(Qt::DisplayRole).toString().toStdString() + ";";
			writer << model->item(i, 17)->data(Qt::DisplayRole).toString().toStdString() + ";\n";
			writer << model->item(i, 0)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 1)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 15)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 19)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 20)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 21)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			writer << model->item(i, 22)->data(Qt::DisplayRole).toString().toStdString() + "\n";
			string file1 = "test" + to_string(i - 1);
			std::ofstream _file1(file1 + ".txt", std::ios_base::out);
			if (_file1)
			{
				_file1 << writer.str();
			}
			_file1.close();
		}
	}
	if (DeviceName == "СКТ-232Б")
	{
		system("Converter.exe SKT");
	}
	if (DeviceName == "45Д20-2")
	{
		system("Converter.exe 45D20");
	}
	fileNum = 0;
	loop = 1;

	model->clear();
	CreateTable();
}

void StepWindow::CreateTable()
{
	if (DeviceName == "45Д20-2")
	{
		loop = 0;
		for (int i = 0; i < 23; i++)
		{
			model->setItem(0, i, new QStandardItem(QString::fromStdString("")));
		}
		int angleTable = 40;
		for (int i = 2; i < 18; i++)
		{
			if (angleTable == 0)angleTable -= 5;
			model->setHeaderData(i, Qt::Horizontal, QString::fromStdString(to_string(angleTable)));
			angleTable -= 5;
		}
		model->setHeaderData(0, Qt::Horizontal, QString::fromStdString("Iвых"));
		model->setHeaderData(1, Qt::Horizontal, QString::fromStdString("Uмин"));
		model->setHeaderData(18, Qt::Horizontal, QString::fromStdString("Uвых"));
		model->setHeaderData(19, Qt::Horizontal, QString::fromStdString("Uкомп/Uвых"));
		model->setHeaderData(20, Qt::Horizontal, QString::fromStdString("Класс"));
		model->setHeaderData(21, Qt::Horizontal, QString::fromStdString("Uвых при 40град"));
		model->setHeaderData(22, Qt::Horizontal, QString::fromStdString("Номер изделия"));
	}
	if (DeviceName == "СКТ-232Б")
	{
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

		
		model->setItem(0, 0, new QStandardItem(QString::fromStdString("<15")));
		model->setItem(0, 1, new QStandardItem(QString::fromStdString(">7")));
		model->setHeaderData(0, Qt::Horizontal, QString::fromStdString("I (mA)"));//Usr / 10
		model->setHeaderData(1, Qt::Horizontal, QString::fromStdString("Umax(V)"));//110 - 180grad max
		int k = 0;
		for (auto t : Punkts)
		{
			model->setItem(0, k + 2, new QStandardItem(QString::fromStdString(to_string(Diap[Ind[k]][0]))));
			if (t.first == "к1")model->setHeaderData(12, Qt::Horizontal, QString::fromStdString(t.first));
			else if (t.first == "к2")model->setHeaderData(13, Qt::Horizontal, QString::fromStdString(t.first));
			else if (t.first == "к3")model->setHeaderData(14, Qt::Horizontal, QString::fromStdString(t.first));
			else if (t.first == "л")model->setHeaderData(11, Qt::Horizontal, QString::fromStdString(t.first));
			else model->setHeaderData(k+2, Qt::Horizontal, QString::fromStdString(t.first));
			k++;
		}
		k=15;
		model->setItem(0, k, new QStandardItem("<10|<20"));
		model->setHeaderData(k, Qt::Horizontal, "Электромаг.\n Ассиметрия");
		k++;
		model->setItem(0, k, new QStandardItem("<10|<20"));
		model->setHeaderData(k, Qt::Horizontal, "Нера-во коэф.\n Ротора");
		k++;
		model->setItem(0, k, new QStandardItem("<10|<20"));
		model->setHeaderData(k, Qt::Horizontal, "Нера-во коэф.\n Статора");
		k++;
		model->setItem(0, k, new QStandardItem("<10|<20"));
		model->setHeaderData(k, Qt::Horizontal, "Нера-во\nсопротивления");
		k++;
		model->setItem(0, k, new QStandardItem("<150|<200"));
		model->setHeaderData(k, Qt::Horizontal, "Квадратур.\nнапряжение(мВ)");
		k++;
		model->setItem(0, k, new QStandardItem("СТУ"));
		model->setHeaderData(k, Qt::Horizontal, "Фаза");
		k++;
		model->setItem(0, k, new QStandardItem(""));
		model->setHeaderData(k, Qt::Horizontal, "Номер изделия");
		k++;
	}
}

void StepWindow::DeleteRow()
{
	if (!DialogDel())return;
	model->removeRow(rowDel-1);
	fileNum--;
	loop--;
}

void StepWindow::Serialization(string file, vector<Data> cont)
{
	stringstream ss;
	for (int i = 0; i < cont.size(); i++)
	{
		
		if (DeviceName == "45Д20-2")
		{
			ss << cont[i].angle << " " << cont[i].min << "'\t\t";// << cont[i].sec << "'" << "'\t";
			ss << cont[i].V1 << "\t\t" << cont[i].V2 << "\t\t" << cont[i].I << "\n";
		}
		if (DeviceName == "СКТ-232Б")
		{
			ss << cont[i].angle << " " << cont[i].min << "'\t\t";// << cont[i].sec << "'" << "'\t";
			ss << cont[i].V2 << "\n";
		}

		//ss << cont[i].angleGrad << "       ";
		//ss << cont[i].V1 << "       \n";
		//ss << cont[i].Phase1 << "       ";
		//ss << cont[i].Phase1 << "       ";
		//ss << cont[i].I << "       ";
		//ss << cont[i].IPhase << 
	}
	std::ofstream _file(file + ".txt", std::ios_base::out);
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

bool StepWindow::DialogDel()
{
	bool ok;
	QString text = QInputDialog::getText(this,
		QString::fromUtf8("Ввод"),
		QString::fromUtf8("Введите номер строки для удаления:"),
		QLineEdit::Normal,
		QDir::home().dirName(), &ok);
	rowDel = text.toInt();
	return ok;
}

void StepWindow::AboutProgramm()
{
	QMessageBox box;
	box.setText("Программа измерений программноаппаратного комплекса УПП-1 \nFirmware версия 1.0 \nИдентификационный номер (CRC32): 889DBC3C");
	box.exec();
}

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
		if (cont[i] > 0)return true;
	}
	return false;
}

bool StepWindow::Minus(vector<int> cont)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i] < 0)return true;
	}
	return false;
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

int StepWindow::FindMinV(vector<Data> cont)
{
	float min = cont[0].V2;
	if (DeviceName == "45Д20-2")min = cont[0].V1;
	if (DeviceName == "СКТ-232Б")min = cont[0].V2;
	int idx = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		float tmp;
		if (DeviceName == "45Д20-2")tmp = cont[i].V1;
		if (DeviceName == "СКТ-232Б")tmp = cont[i].V2;
		if (tmp < min)
		{
			min = tmp;
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

int StepWindow::FindAngle(int _angle, int _min, vector<Data> cont)
{
	vector<float> temp;
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].angle == _angle)
		{
			if (cont[i].min == _min)
			{
				return i;
			}
		}
	}
	return 0;
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
	float min;
	if(DeviceName == "45Д20-2") min = cont[i].V1;
	if (DeviceName == "СКТ-232Б") min = cont[i].V2;
	
	int idx = i;
	for (int j = i; j <= cont.size(); j++)
	{
		if (cont[j].angle > maxAngle)break;
		if (cont[j].angle == maxAngle && cont[j].min > 0)break;

		float temp;
		if (DeviceName == "45Д20-2") temp = cont[j].V1;
		if (DeviceName == "СКТ-232Б") temp = cont[j].V2;

		if (temp < min)
		{
			min = temp;
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

float StepWindow::FindUex(vector<int> cont)
{
	int max = cont[0];
	float result = 0;
	int indexMax = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i] > max)
		{
			max = cont[i];
			indexMax = i;
		}
	}
	result = max;
	max = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i] > max && i != indexMax)
		{
			max = cont[i];
		}
	}
	result += max;
	return result / 48.f;
}

float* StepWindow::VectorInMassV(vector<Data> cont,bool V1)
{
	float* V = new float[cont.size()];
	for (int i = 0; i < cont.size(); i++)
	{
		if(V1)V[i] = cont[i].V1;
		else V[i] = cont[i].V2;
	}
	return V;
}

float StepWindow::AverageI(vector<Data> cont)
{
	float sum = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		sum += cont[i].I;
	}
	sum /= cont.size();
	return sum;
}

vector<Data> StepWindow::Average(vector<Data> cont)
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
		int k = 0;
		for (int j = i; j < cont.size(); j++)
		{
			if (currentAngle == Angle[j] && currentMin == Min[j])
			{
				sum += V[j];
				sum2 += V2[j];
				sumI += cont[j].I;
				k++;
			}
		}
		sum /= k;
		sum2 /= k;
		sumI /= k;

		Data dt;
		dt.angle = currentAngle;
		dt.min = currentMin;
		dt.V2 = sum2;
		if (DeviceName == "45Д20-2")
		{
			dt.V1 = sum;
			dt.V2 = sum2;
			dt.I = sumI;
		}
		temp.push_back(dt);
	}
	return temp;
}

vector<Data> StepWindow::EraseErrors(vector<Data> cont)
{
	vector<Data> temp;
	temp.push_back(cont[0]);
	for (int i = 1; i < cont.size(); i++)
	{
		if (DeviceName == "45Д20-2")
		{
			if (abs(temp[temp.size() - 1].V1 - cont[i].V1) < 60);
			temp.push_back(cont[i]);
		}
		if (DeviceName == "СКТ-232Б")
		{
			if (abs(temp[temp.size() - 1].V2 - cont[i].V2) < 60);
			temp.push_back(cont[i]);
		}
	}
	vector<Data> result;
	for (int i = 3; i < temp.size() - 2; i++)
	{
		if (DeviceName == "45Д20-2")
		{
			float val1 = (temp[i - 1].V1 + temp[i - 2].V1) / 2;
			float val2 = (temp[i + 1].V1 + temp[i + 2].V1) / 2;
			float delta = (val1 + val2) / 10;
			if (val1 < 50)delta = 5;
			if (abs(temp[i].V1 - val1) < delta || abs(temp[i].V1 - val2) < delta)
				result.push_back(temp[i]);
		}
		if (DeviceName == "СКТ-232Б")
		{
			float val1 = (temp[i - 1].V2 + temp[i - 2].V2) / 2;
			float val2 = (temp[i + 1].V2 + temp[i + 2].V2) / 2;
			float delta = (val1 + val2) / 10;
			if (val1 < 50)delta = 5;
			if (abs(temp[i].V2 - val1) < delta || abs(temp[i].V2 - val2) < delta)
				result.push_back(temp[i]);
		}
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

vector<Data>  StepWindow::OffsetToZero(vector<Data> cont)
{
	int indMin = FindMinV(cont);
	int tmp = cont[indMin].angle;
	int tmpMin = cont[indMin].min;
	for (int i = 0; i < cont.size(); i++)
	{
		int angleEx = cont[i].angle * 60 + cont[i].min;
		int angleRes = angleEx - (tmp * 60 + tmpMin);
		cont[i].angle = angleRes / 60;
		cont[i].min = angleRes % 60;
		if (cont[i].min < 0 && cont[i].angle < 0)cont[i].min *= -1;
	}
	return cont;
}

Data StepWindow::Read()
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

Data StepWindow::FindNearestAngle(float V, int _angle, vector<Data> cont)
{
	int ind = 0;
	vector<Data> temp;
	for (int i = 0; i < cont.size(); i++)
	{
		int tmpAng = _angle < 0 ? _angle + 1 : _angle - 1;
		if (cont[i].angle == _angle || cont[i].angle == tmpAng)
		{
			if (abs(cont[i].V1 - V) < min)
			{
				temp.push_back(cont[i]);
			}
		}
	}
	float min = V;
	for (int i = 0; i < temp.size(); i++)
	{
		if (abs(temp[i].V1 - V) < min)
		{
			min = abs(temp[i].V1 - V);
			ind = i;
		}
	}
	Serialization("Near", temp);
	return temp[ind];
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