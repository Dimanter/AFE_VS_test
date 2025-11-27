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

	CreateWindow(device);
	window->showMaximized();

	timer->setInterval(1.f);
	connect(timer, SIGNAL(timeout()), this, SLOT(Update()));

	testTimer->setInterval(1.f);
	connect(testTimer, SIGNAL(timeout()), this, SLOT(Test()));
	
	CreateTable();
	LoadWindow(device);
	qApp->setPalette(darkPalette);
	QApplication::exec();
}

void StepWindow::Test232BComplete()
{
	Stop();
	stringstream writer;
	testTimer->stop();
	//*Запись данных в файл
	data = Erase(data);
	Serialization("Data0", data);
	//*
	bool flag = false;//Флаг смены направления(возрастание/убывание)
	bool flagResist = false;//Флаг замеров квадратурного напряжения
	bool endDiap = false;//Флаг конца диапазона
	
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
	string IndPunkts[13] = { "а","б","в","г","д","е","ж","з","и","л","к1","к2","к3" };//Пункты комутации
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

				Uav += adt[i].V2;
				UavSize++;

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
		string tmp = "Проверьте правильность подключения " + to_string(allData.size());
		QMessageBox::warning(window, "Error", QString::fromStdString(tmp));
		return;
	}
	//*Убираем ошибки при переключении схемы комутации и усредняем данные
	Quad = EraseErrors(Quad);
	Serialization("Quad", Quad);
	int maxU = FindMaxV(MaxU);//Индекс максимального напряжения
	Uav /= UavSize;
	float Isr = Uav / 1000;//Сила тока
	try
	{
		for (int i = 0; i < allData.size(); i++)
		{
			allData[i] = EraseErrors(allData[i]);
			allData[i] = Average(allData[i]);
			allData[i] = Erase(allData[i]);
			allData[i] = Erase(allData[i]);
			MinV.push_back(FindMinV(allData[i]));
			if (allData[i][MinV[i]].V2 > MinCoef)
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
	Quad[QuadM].V2 = Quad[QuadM].V2 * QuadCoef;
	Isr = Isr * ICoef;
	MaxU[maxU].V2 = MaxU[maxU].V2 * UCoef;
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
	elAssimetry <= 10 ? elText->setBackground(green) : (elAssimetry <= 17 ? elText->setBackground(yellow) : elText->setBackground(red));
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
	string file1 = "232B" + to_string(fileNum);
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
	try
	{
		Serialization("data1", data);
		data = EraseErrors(data);
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}

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

		AngleRes.push_back(angleEx);
		ErrRate.push_back(FindNearestAngle(angleEx, -40, data));
		k++;
	}
	catch (const std::exception&)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных, проверьте правильность установки датчика.");
		return;
	}

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
	std::ofstream _file("45D20" + to_string(fileNum) + ".txt", std::ios_base::out);
	if (_file)
	{
		_file << writer.str();
	}
	_file.close();
	loop++;
	fileNum++;
}

void StepWindow::Test265DComplete()
{
	Stop();
	testTimer->stop();
	stringstream writer;//потоковая запись результатов измерений в файл
	stringstream helper;
	//*Запись данных в файл
	
	//data = ReadFromFile("Data0");
	bool endDiap = true;
	bool endLoop = false;
	vector<vector<Data>> MinAngl;
	vector<Data> adt = data;//Контенер данных с устройства
	vector<Data> MinU;//Напряжение в каждом диапазоне
	vector<Data> elCur;
	vector<Data> Quad;
	vector<int> Grad;
	float MaxQuad = 0;
	float MaxU1 = 0;
	float MaxU2 = 0;
	float UMin = 0;
	float MaxTok = 0;
	int MaxMin = 0;
	try
	{
		for (int i = 0; i < adt.size(); i++)
		{
			if (!endLoop)
			{
				if (adt[i].angle > 20 && adt[i].angle < 45)
				{
					elCur.push_back(adt[i]);
				}
				else if (adt[i].angle > 65 && adt[i].angle < 90 || adt[i].angle > 110 && adt[i].angle < 135 
					 || adt[i].angle > 155 && adt[i].angle < 180 || adt[i].angle > 200 && adt[i].angle < 225 
					 || adt[i].angle > 245 && adt[i].angle < 270 || adt[i].angle > 290 && adt[i].angle < 315 || adt[i].angle > 335)
				{
					Quad.push_back(adt[i]);
				}


				if (adt[i].angle >= 359) endLoop = true;

				for (int j = 0; j < 8; j++)
				{
					if (adt[i].angle >= Diap265[j][0] && adt[i].angle <= Diap265[j][1])
					{
						if (adt[i].angle == Diap265[j][0])
						{
							if (!MinU.empty())
							{
								MinU.clear();
								continue;
							}
						}
						if (adt[i].angle == Diap265[j][1] && endDiap)
						{
							if (MinU.size() < 10)continue;
							allData.push_back(MinU);
							MinU.clear();
							endDiap = false;
							continue;
						}
						else
						{
							if (adt[i].angle != Diap265[j][1])
								endDiap = true;
						}
						MinU.push_back(adt[i]);
					}
				}
			}
			else
			{
				Quad.push_back(adt[i]);
			}
		}
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить разделение данных, проверьте правильность установки датчика.");
		return;
	}
	try
	{
		if (allData.size() < 10)
		{
			QMessageBox::warning(window, "Error", "Не удалось выполнить разделение данных, проверьте правильность установки датчика.");
			return;
		}
		for (int i = 0; i < allData.size(); i++)
		{
			if (i == 3 || i == 4)continue;
			MinAngl.push_back(allData[i]);
		}
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных.");
		return;
	}
	try
	{
		Quad = EraseErrors(Quad);
		Quad = Erase(Quad);
		Quad = EraseErrors(Quad);
		Quad = Erase(Quad);
		elCur = EraseErrors(elCur);
		elCur = Erase(elCur);
		allData[3] = EraseErrors(allData[3]);
		allData[4] = EraseErrors(allData[4]);
		allData[3] = Erase(allData[3]);
		allData[4] = Erase(allData[4]);
		
		for (int i = 0; i < 8; i++)
		{
			MinAngl[i] = EraseErrors(MinAngl[i]);
			MinAngl[i] = Average(MinAngl[i]);
			MinAngl[i] = Erase(MinAngl[i]);
			MinAngl[i] = Erase(MinAngl[i]);
			MinV.push_back(FindMinV(MinAngl[i]));
			if (MinAngl[i][MinV[i]].V2 > MaxMin)MaxMin = MinAngl[i][MinV[i]].V2;
			helper << MinAngl[i][MinV[i]].V2 << "\t";
			if (MinAngl[i][MinV[i]].V2 > MinCoef)
			{
				QMessageBox::warning(window, "Error", "Umin > 30 \n" +
					QString::fromStdString(to_string(MinAngl[i][MinV[i]].angle) + " " + to_string(MinAngl[i][MinV[i]].min) + "'"
						+ " " + to_string(MinAngl[i][MinV[i]].V2)));
				//return;
			}
		}
		helper << "\n" << allData.size();
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось выполнить обработку данных.");
		return;
	}
	
	try
	{
		MaxQuad = FindMaxV(Quad);
		MaxU1 = FindMaxV(allData[3]);
		MaxU2 = FindMaxV(allData[4]);
		MaxTok = FindMaxV(elCur);
		writer << "0';";
		model->setItem(loop, 1, new QStandardItem(QString::fromStdString("0'")));
		for (int i = 1; i < 8; i++)
		{
			int angleT = MinAngl[i][MinV[i]].angle - Diap265[i][0];
			int minT = MinAngl[i][MinV[i]].min;

			int angleM = MinAngl[0][MinV[0]].angle * 60 + MinAngl[0][MinV[0]].min;
			int delta = (angleT * 60 + minT) - angleM;
			angleT = delta / 60;
			minT = delta % 60;

			model->setItem(loop, i+1, new QStandardItem(QString::fromStdString(to_string(minT) + "'")));
			writer << minT << "';";
			Grad.push_back(minT);
		}
		writer << "\n";
		UMin = (MaxMin * 3.6) / (allData[3][MaxU1].V2 / 1000);
	}
	catch (...)
	{
		QMessageBox::warning(window, "Error", "Не удалось вычислить отклонение роторов, проверьте правильность установки датчика.");
		return;
	}

	float temp = elCur[MaxTok].V2 / 10;
	writer << UMin << "\n" << allData[3][MaxU1].V2/1000 << "\n" << allData[4][MaxU2].V2/1000 << "\n" << Quad[MaxQuad].V2 << "\n" << elCur[MaxTok].V2 / 10 << "\n" << NumberDevice.toStdString();
	QStandardItem* QuadText = new QStandardItem(QString::fromStdString(to_string((Quad[MaxQuad].V2)).erase(5, 9)));
	QStandardItem* MaxU1Text = new QStandardItem(QString::fromStdString(to_string((allData[3][MaxU1].V2/1000)).erase(4, 9)));
	QStandardItem* MaxU2Text = new QStandardItem(QString::fromStdString(to_string((allData[4][MaxU2].V2)/1000).erase(6, 9)));
	QStandardItem* UMinText = new QStandardItem(QString::fromStdString(to_string((UMin)).erase(3, 9)));
	QStandardItem* MaxTokText = new QStandardItem(QString::fromStdString(to_string((temp)).erase(4, 9)));

	Quad[MaxQuad].V2 <= 100 ? QuadText->setBackground(green) : (Quad[MaxQuad].V2 <= 200 ? QuadText->setBackground(yellow) : QuadText->setBackground(red));
	allData[3][MaxU1].V2/1000 <= 34 ? MaxU1Text->setBackground(red) : allData[3][MaxU1].V2/1000 >= 38 ? MaxU1Text->setBackground(red) : MaxU1Text->setBackground(green);
	allData[4][MaxU2].V2/1000 <= 19 ? MaxU2Text->setBackground(red) : allData[4][MaxU2].V2/1000 >= 25 ? MaxU2Text->setBackground(red) : MaxU2Text->setBackground(green);
	UMin <= 5 ? UMinText->setBackground(green) : UMin <= 7.5 ? UMinText->setBackground(yellow) : UMinText->setBackground(red);
	temp <= 38 ? MaxTokText->setBackground(red) : elCur[MaxTok].V2 <= 52 ? MaxTokText->setBackground(green) : MaxTokText->setBackground(red);

	model->setItem(loop, 0, UMinText);
	model->setItem(loop, 9, MaxU1Text);
	model->setItem(loop, 10, MaxU2Text);
	model->setItem(loop, 11, QuadText);
	model->setItem(loop, 12, MaxTokText);

	for (int i = 0; i < MinAngl.size(); i++)
	{
		Serialization("Dt" + to_string(i), MinAngl[i]);
	}
	Serialization("MaxU1", allData[3]);
	Serialization("MaxU2", allData[4]);
	Serialization("Quad", Quad);
	Serialization("Tok", elCur);// tok/10
	
	string file1 = "265D" + to_string(fileNum);
	std::ofstream _file1(file1 + ".txt", std::ios_base::out);
	if (_file1)
	{
		_file1 << writer.str();
	}
	_file1.close();

	std::ofstream _file2("helper.txt", std::ios_base::out);
	if (_file2)
	{
		_file2 << helper.str();
	}
	_file2.close();

	loop++;
	fileNum++;
}//u min = findmax(u min) * 3.6 / MaxU1

void StepWindow::Test()
{
	if (work->ProccessStatus())
	{
		if (DeviceName == "СКТ-232Б")
		{
			data = Erase(data);
			Test232BComplete();
		}
		else if (DeviceName == "45Д20-2")
		{
			Test45D20Complete();
		}
		else if (DeviceName == "СКТ-265Д")
		{
			data = Erase(data);
			Serialization("Data0", data);
			Test265DComplete();
		}
	}
	float ref = work->measure->refAngle;
	ConvertAngle(ref);
	if (Contains(data, ref))return;
	data.push_back(Read());
}

void StepWindow::setupDevice45D20(QStandardItemModel* model)
{
	for (int i = 0; i < 23; i++) {
		model->setItem(0, i, new QStandardItem(QString::fromStdString("")));
	}

	QStringList headers = { "Iвых", "Uмин" };
	int angleTable = 40;
	for (int i = 2; i < 18; i++) {
		headers.append(QString::fromStdString(to_string(angleTable)));
		angleTable -= 5;
	}
	headers.append("Uвых");
	headers.append("Uкомп/Uвых");
	headers.append("Класс");
	headers.append("Uвых при 40град");
	headers.append("Номер изделия");

	setModelHeaders(model, headers);
}

void StepWindow::setupDeviceSKT232B(QStandardItemModel* model)
{
	// Заполнение пунктов комутации
	Punkts =
	{
		{"а", 0}, {"б", 1}, {"в", 6}, {"г", 7}, {"д", 9},
		{"е", 10}, {"ж", 11}, {"з", 12}, {"и", 3}, {"л", 5},
		{"к1", 2}, {"к2", 8}, {"к3", 4}
	};

	model->setItem(0, 0, new QStandardItem("<15"));
	model->setItem(0, 1, new QStandardItem(">7"));
	setModelHeaders(model, { "I (mA)", "Umax(V)" });

	for (int i = 0; i < 13; i++)
	{
		model->setItem(0, i + 2, new QStandardItem(QString::fromStdString(to_string(Diap[Ind[i]][0]))));
	}

	int k = 0;
	for (const auto& t : Punkts)
	{


		// Установка заголовков для специальных пунктов
		if (t.first == "к1") model->setHeaderData(12, Qt::Horizontal, QString::fromStdString(t.first));
		else if (t.first == "к2") model->setHeaderData(13, Qt::Horizontal, QString::fromStdString(t.first));
		else if (t.first == "к3") model->setHeaderData(14, Qt::Horizontal, QString::fromStdString(t.first));
		else if (t.first == "л") model->setHeaderData(11, Qt::Horizontal, QString::fromStdString(t.first));
		else model->setHeaderData(k + 2, Qt::Horizontal, QString::fromStdString(t.first));

		k++;
	}

	// Заполнение дополнительных заголовков
	QStringList additionalHeaders =
	{
		"<10|<17", "<10|<20", "<10|<20", "<10|<20", "<150|<200", "СТУ", ""
	};
	QStringList additionalHeaderNames =
	{
		"Электромаг.\nАссиметрия", "Нера-во коэф.\nРотора",
		"Нера-во коэф.\nСтатора", "Нера-во\nсопротивления",
		"Квадратур.\nнапряжение(мВ)", "Фаза", "Номер изделия"
	};
	k += 2;
	for (int i = 0; i < additionalHeaders.size(); ++i)
	{
		model->setItem(0, k, new QStandardItem(additionalHeaders[i]));
		model->setHeaderData(k, Qt::Horizontal, additionalHeaderNames[i]);
		k++;
	}
}

void StepWindow::setupDeviceSKT265D(QStandardItemModel* model)
{
	for (int i = 0; i < 23; i++) {
		model->setItem(0, i, new QStandardItem(QString::fromStdString("")));
	}
	QStringList additionalHeaders =
	{
		"U min \nКл. 0,1 <= 5\nКл. 0,2 <= 7,5",
		"0", "45", "90","135","180", "225", "270", "315",
		"U max\nХолостой\n36+-2", "Umax\nПри нагрузке\n22+-3", "Квадратурное\nнапряжение\nКл0,1<100\n,Кл0,2<200","I потр.\n45+-7","Класс"
	};

	for (int i = 0; i < additionalHeaders.size(); i++)
	{
		model->setHeaderData(i, Qt::Horizontal, additionalHeaders[i]);
	}
}

void StepWindow::CreateTable()
{
	loop = 0;
	if (DeviceName == "45Д20-2") {
		setupDevice45D20(model);
	}
	else if (DeviceName == "СКТ-232Б") {
		setupDeviceSKT232B(model);
	}
	else if (DeviceName == "СКТ-265Д") {
		setupDeviceSKT265D(model);
	}
}

void StepWindow::DeleteRow()
{
	if (!DialogDel())return;
	model->removeRow(rowDel - 1);
	fileNum--;
	loop--;
}

void StepWindow::SwitchDevice()
{
	DeviceName = CurrentDevice->itemText(CurrentDevice->currentIndex());
	CreateTable();
}

void StepWindow::Update()
{
	StepThreading();
	if (!work->port->isOpen())Disconnect();
}

void StepWindow::StepThreading()
{
	ConvertAngle(work->measure->refAngle);
	std::ostringstream stream;
	stream << std::fixed << std::setprecision(1) << secDec;
	std::string roundedString = stream.str();
	textAngle->setText(QString::fromStdString(to_string(angle) + " " + to_string(min) + "' " + roundedString + "'" + "'"));
	I->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.RMS)));
	PhaseV2->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V2.Phase)));
	PhaseV1->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.Phase)));
	RmsV1->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).V1.RMS)));
	if (DeviceName == "45Д20-2")
	{
		std::ostringstream U;
		U << std::fixed << std::setprecision(1) << (measures_t(work->measure->Result).V1.RMS);
		std::string roundedString = U.str();
		RmsV2->setText(QString::fromStdString(roundedString));
	}
	if (DeviceName == "СКТ-232Б")
	{
		std::ostringstream U;
		U << std::fixed << std::setprecision(1) << (measures_t(work->measure->Result).V2.RMS);
		std::string roundedString = U.str();
		RmsV2->setText(QString::fromStdString(roundedString));
	}
	if(DeviceName == "СКТ-265Д")
	{
		std::ostringstream U;
		U << std::fixed << std::setprecision(1) << (measures_t(work->measure->Result).V2.RMS);
		std::string roundedString = U.str();
		RmsV2->setText(QString::fromStdString(roundedString));
	}
	IPhase->setText(QString::fromStdString(to_string(measures_t(work->measure->Result).I.Phase)));
}

void StepWindow::CreateWindow(QString device)
{
	//Main window ================================================================================
	QString title = "Meter " + device;
	window->setWindowTitle(title);
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

	CurrentDevice = new QComboBox;
	CurrentDevice->setStyleSheet("QComboBox{"
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
	CurrentDevice->addItem("45Д20-2");
	CurrentDevice->addItem("СКТ-232Б");
	CurrentDevice->addItem("СКТ-265Д");
	RefreshComBox();

	btnRefreshPort = new QPushButton("Обновить");
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
	btnStart->setMinimumWidth(200);
	btnStart->setMinimumHeight(50);
	btnStart->setEnabled(false);
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

	btnStop = new QPushButton("Стоп");
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
	
	toolBar->addWidget(aboutButton);

	QVBoxLayout* PortV = new QVBoxLayout();
	QVBoxLayout* StartV = new QVBoxLayout();
	QVBoxLayout* ConnectV = new QVBoxLayout();
	QVBoxLayout* AngleV = new QVBoxLayout();
	QVBoxLayout* RmsV = new QVBoxLayout();
	QVBoxLayout* IV = new QVBoxLayout();

	QHBoxLayout* PortH = new QHBoxLayout();
	QHBoxLayout* ComboBoxes = new QHBoxLayout();
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
	ComboBoxes->addWidget(comboPort1);
	PortV->addLayout(ComboBoxes);
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

	QObject::connect(btnRefreshPort, &QPushButton::clicked, this, &StepWindow::RefreshComBox);
	QObject::connect(btnReport, &QPushButton::clicked, this, &StepWindow::StartUpConverter);
	QObject::connect(btnDelete, &QPushButton::clicked, this, &StepWindow::DeleteRow);
	QObject::connect(btnConnect, &QPushButton::clicked, this, &StepWindow::Connect);
	QObject::connect(btnDisconnect, &QPushButton::clicked, this, &StepWindow::Disconnect);
	QObject::connect(btnMonitor, &QPushButton::clicked, this, &StepWindow::StartMonitor);
	QObject::connect(btnStop, &QPushButton::clicked, this, &StepWindow::Stop);
	QObject::connect(aboutButton, &QPushButton::clicked, this, &StepWindow::AboutProgramm);
	QObject::connect(btnStart, &QPushButton::clicked, this, &StepWindow::Start);

	window->setLayout(layoutMainVer);
}

void StepWindow::LoadWindow(QString device)
{

	try {
		std::ifstream _file("Init.txt");
		string text;
		getline(_file, text);
		getline(_file, text);
		getline(_file, text);
		QuadCoef = stof(text);
		getline(_file, text);
		ICoef = stof(text);
		getline(_file, text);
		UCoef = stof(text);
		getline(_file, text);
		MinCoef = stof(text);
		_file.close();
	}
	catch (const std::exception&)
	{
		QMessageBox msgBox;
		msgBox.setText("Ошибка: Не удалось считать настройки!\nБудут выставлены настройки по умолчанию.");
		msgBox.exec();
	}

	int i = 0;
	if (device == "СКТ-232Б")
	{
		while (1)
		{
			std::ifstream _file("232B" + to_string(i) + ".txt");
			if (_file.is_open())
			{
				int k = 2;
				string tmp = "";
				string text;

				getline(_file, text);
				for (int j = 0; j < text.size(); j++)
				{
					if (text[j] == ';')
					{
						model->setItem(i, k, new QStandardItem(QString::fromStdString(tmp)));
						k++;
						tmp = "";
					}
					else
					{
						tmp += text[j];
					}
				}
				getline(_file, text);
				model->setItem(i, 0, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 1, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 15, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 16, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 17, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 18, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 19, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 20, new QStandardItem(QString::fromStdString("СТУ")));
				model->setItem(i, 21, new QStandardItem(QString::fromStdString(text)));
			}
			else
			{
				_file.close();
				fileNum = i;
				loop = i;
				return;
			}
			i++;
		}
		return;
	}
	else if (device == "45Д20-2")
	{
		while (1)
		{
			std::ifstream _file("45D20" + to_string(i) + ".txt");
			if (_file.is_open())
			{
				int k = 2;
				string tmp = "";
				string text;

				getline(_file, text);
				for (int j = 0; j < text.size(); j++)
				{
					if (text[j] == ';')
					{
						model->setItem(i, k, new QStandardItem(QString::fromStdString(tmp)));
						k++;
						tmp = "";
					}
					else
					{
						tmp += text[j];
					}
				}
				getline(_file, text);
				model->setItem(i, 0, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 1, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 18, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 19, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 21, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 20, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 22, new QStandardItem(QString::fromStdString(text)));
			}
			else
			{
				_file.close();
				fileNum = i;
				return;
			}
			i++;
		}
		return;
	}
	else if (device == "СКТ-265Д")
	{
		while (1)
		{
			std::ifstream _file("265D" + to_string(i) + ".txt");
			if (_file.is_open())
			{
				int k = 1;
				string tmp = "";
				string text;

				getline(_file, text);
				for (int j = 0; j < text.size(); j++)
				{
					if (text[j] == ';')
					{
						model->setItem(i, k, new QStandardItem(QString::fromStdString(tmp)));
						k++;
						tmp = "";
					}
					else
					{
						tmp += text[j];
					}
				}
				getline(_file, text);
				model->setItem(i, 0, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 9, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 10, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 11, new QStandardItem(QString::fromStdString(text)));
				getline(_file, text);
				model->setItem(i, 12, new QStandardItem(QString::fromStdString(text)));
			}
			else
			{
				_file.close();
				fileNum = i;
				loop = i;
				return;
			}
			i++;
		}
		return;
	}
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
	if (!work->isOpen())
	{
		DiactivateButton();
		return;
	}
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
	textAngle->clear();
	work->Stop();
}

void StepWindow::Start()
{
	if (fileNum > 48)
	{
		QMessageBox::warning(window, "Error", "Документ переполнен. Создайте отчёт.");
		return;
	}
	if (work->stage != Work::Stage::Idle)return;
	if (!Dialog())return;
	if (DeviceName == "45Д20-2")
	{
		work->test45D20();
	}
	else if (DeviceName == "СКТ-232Б")
	{
		work->testSKT232B();
	}
	else if (DeviceName == "СКТ-265Д")
	{
		work->testSKT265D();
	}
	work->processRun();
	testTimer->start();
	data.clear();
	allData.clear();
	MinV.clear();
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

void StepWindow::deleteOldFiles()
{
	system("del test*.txt");
}

void StepWindow::writeDataToFile(QStandardItemModel* model, int fileIndex)
{
	std::stringstream writer;
	// Заполнение данных из модели
	for (int j = 2; j <= 14; j++) 
	{
		writer << model->item(fileIndex, j)->data(Qt::DisplayRole).toString().toStdString() + ";";
	}
	for (int j = 0; j <= 1; j++) 
	{
		writer << model->item(fileIndex, j)->data(Qt::DisplayRole).toString().toStdString() + "\n";
	}
	for (int j = 15; j <= 22; j++) 
	{
		writer << model->item(fileIndex, j)->data(Qt::DisplayRole).toString().toStdString() + "\n";
	}
	// Запись в файл
	std::string fileName = "test" + std::to_string(fileIndex - 1) + ".txt";
	std::ofstream outFile(fileName, std::ios_base::out);
	if (outFile) 
	{
		outFile << writer.str();
	}
	outFile.close();
}

void StepWindow::StartUpConverter()
{
	if (DeviceName == "СКТ-232Б")
	{
		system("Converter.exe SKT");
	}
	else if (DeviceName == "45Д20-2")
	{
		system("Converter.exe 45D20");
	}
	else if (DeviceName == "СКТ-265Д")
	{
		system("Converter.exe SKT265");
	}

	fileNum = 0;
	loop = 1;

	model->clear();
	CreateTable();
}

void StepWindow::setModelHeaders(QStandardItemModel* model, const QStringList& headers)
{
	for (int i = 0; i < headers.size(); ++i) 
	{
		model->setHeaderData(i, Qt::Horizontal, headers[i]);
	}
}

void StepWindow::Serialization(string file, vector<Data> cont)
{
	std::ofstream outFile(file + ".txt");
	stringstream writer;

	for (int i=0;i<cont.size();i++) 
	{
		if (DeviceName == "45Д20-2") 
		{
			writer << cont[i].angle << " " << cont[i].min << "'\t\t"
				<< cont[i].V1 << "\t\t" << cont[i].V2 << "\t\t" << cont[i].I << "\n";
		}
		else if (DeviceName == "СКТ-232Б") 
		{
			writer << cont[i].angle << " " << cont[i].min << "'\t\t"
				<< cont[i].V2 << "\n";
		}
		else if (DeviceName == "СКТ-265Д")
		{
			writer << cont[i].angle << " " << cont[i].min << "'\t\t" << cont[i].V2 << "\n";
				//<< cont[i].sec << "''\t\t" << cont[i].V2 << "\n";
		}
	}
	outFile << writer.str();
	outFile.close();
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
	_sec = _sec * 60.0;//std::trunc(_sec * 60.0);
	angle = grad;
	min = _min;
	sec = _sec;
	secDec = std::round(_sec * 10.0f) / 10.0f;
}

void StepWindow::ConvertAngle(float _angle, bool twin)
{
	float grad, _min, _sec;
	grad = std::trunc(_angle);
	_min = _angle - grad;
	if (twin)
		_min = std::trunc(_min * 120.0);
	_sec = (_angle - grad) * 120.0 - _min;
	_sec = _sec * 60.0;//std::trunc(_sec * 60.0);
	angle = grad;
	min = _min;
	sec = _sec;
	secDec = std::round(_sec * 10.0f) / 10.0f;
}

void StepWindow::AboutProgramm()
{
	QMessageBox box;
	box.setText("Программа измерений программноаппаратного комплекса УПП-1 \nFirmware версия 1.0 \nИдентификационный номер (CRC32): 889DBC3C");
	box.exec();
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
		if (cont[i].angle == _angle)
			if (cont[i].min == _min)
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

bool StepWindow::CheckDiap(int _angle)
{
	for (int i = 0; i < 8; i++)
		if (_angle >= Diap265[i][0] && _angle <= Diap265[i][1])return true;
	return false;
}

int StepWindow::FindMinV(vector<Data> cont)
{
	float min = cont[0].V2;
	if (DeviceName == "45Д20-2")min = cont[0].V1;
	if (DeviceName == "СКТ-232Б")min = cont[0].V2;
	if (DeviceName == "СКТ-265Д")min = cont[0].V2;
	int idx = 0;
	for (int i = 0; i < cont.size(); i++)
	{
		float tmp = 0;
		if (DeviceName == "45Д20-2")tmp = cont[i].V1;
		if (DeviceName == "СКТ-232Б")tmp = cont[i].V2;
		if (DeviceName == "СКТ-265Д")tmp = cont[i].V2;
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
	if (DeviceName == "СКТ-265Д") min = cont[i].V2;
	int idx = i;
	for (int j = i; j <= cont.size(); j++)
	{
		if (cont[j].angle > maxAngle)break;
		if (cont[j].angle == maxAngle && cont[j].min > 0)break;

		float temp;
		if (DeviceName == "45Д20-2") temp = cont[j].V1;
		if (DeviceName == "СКТ-232Б") temp = cont[j].V2;
		if (DeviceName == "СКТ-265Д") temp = cont[j].V2;
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

float* StepWindow::VectorInMassV(vector<Data> cont, bool V1)
{
	float* V = new float[cont.size()];
	for (int i = 0; i < cont.size(); i++)
	{
		if (V1)V[i] = cont[i].V1;
		else V[i] = cont[i].V2;
	}
	return V;
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
		else if (DeviceName == "СКТ-232Б")
		{
			if (abs(temp[temp.size() - 1].V2 - cont[i].V2) < 60);
			temp.push_back(cont[i]);
		}
		else if (DeviceName == "СКТ-265Д")
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
		else if (DeviceName == "СКТ-232Б")
		{
			float val1 = (temp[i - 1].V2 + temp[i - 2].V2) / 2;
			float val2 = (temp[i + 1].V2 + temp[i + 2].V2) / 2;
			float delta = (val1 + val2) / 10;
			if(val1 > 1000) delta = (val1 + val2) / 30;
			if (val1 < 50)delta = 5;
			if (abs(temp[i].V2 - val1) < delta || abs(temp[i].V2 - val2) < delta)
				result.push_back(temp[i]);
		}
		else if (DeviceName == "СКТ-265Д")
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

Data StepWindow::Read()
{
	Data temp;

	temp.angle = angle;
	temp.min = min;
	temp.sec = sec;
	temp.angleGrad = work->measure->refAngle;
	temp.V1 = measures_t(work->measure->Result).V1.RMS;
	if (DeviceName == "СКТ-265Д")
		temp.V2 = measures_t(work->measure->Result).V2.RMS * 3.86;
	else
		temp.V2 = measures_t(work->measure->Result).V2.RMS;
	temp.Phase1 = measures_t(work->measure->Result).V1.Phase;
	temp.Phase2 = measures_t(work->measure->Result).V2.Phase;
	temp.I = measures_t(work->measure->Result).I.RMS;
	temp.IPhase = measures_t(work->measure->Result).I.Phase;

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