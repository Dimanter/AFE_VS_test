#include "Board.hpp"
#include "DSP.hpp"
template <std::floating_point T>
/*
* @brief Конвертирование в строку
*/
auto convert(T val)
{
	std::stringstream ss;
	const bool        sign = val < 0;
	val = std::abs(val);
	T grad = std::trunc(val);
	T min = val - grad;
	min = std::trunc(min * 60.0);
	T sec = (val - grad) * 60.0 - min;
	sec = std::trunc(sec * 60.0);
	if (sign) {
		ss << "-";
	}
	if (grad > 0 || val == 0) {
		ss << grad << "\u00B0";
	}
	if (min > 0 || sec > 0) {
		ss << min << "\'";
	}
	if (sec > 0) {
		ss << sec << "\"";
	}
	return QString(ss.str().c_str());
}

/*
* @brief Установить параметры
* @param agnel Угол
* @param res Указатель на экземпляр класса измерения
*/
void BoardGraphicsItem::setParameters(float angle, const measures_t& res)
{
	/*
		Лир - перевод угла в минуты / секунды
	*/
	float grad, min, sec;
	grad = std::trunc(angle);
	min = angle - grad;
	min = std::trunc(min * 60.0);
	sec = (angle - grad) * 60.0 - min;
	sec = std::trunc(sec * 60.0);

	/*
		Запись результатов измерения
	*/
	Data.push_back(DataAnalyze{ angle,res.V1.RMS,res.V2.RMS,res.V1.Phase,res.V2.Phase,res.I.Amp,res.I.Phase });

	bufV1.push(res.V1.RMS);
	bufVPhase1.push(res.V1.Phase);
	bufV2.push(res.V2.RMS);
	bufVPhase2.push(res.V2.Phase);
	bufI1.push(res.I.Amp);
	bufIPhase1.push(res.I.Phase);

	textRef->setHtml(QString("<big><bold>&alpha;</bold></big>: %1˚ %2\' %3\"").arg(static_cast<uint32_t>(grad), 3).arg(static_cast<uint32_t>(min), 2).arg(static_cast<uint32_t>(sec), 2));
	textI1->setHtml(QString("<bold>I</bold>: %1").arg(bufI1.apply(DSP::mean), 5, 'f', 3));
	textV1->setHtml(QString("<bold>U</bold>: %1").arg(bufV1.apply(DSP::mean), 5, 'f', 3));
	textV2->setHtml(QString("<bold>U</bold>: %1").arg(bufV2.apply(DSP::mean), 5, 'f', 3));
}

void BoardGraphicsItem::DataScience()
{
	Data = Average();
	Offset();
	vector<float> AngleRes;
	
	float angleEx = FindAngle(-40);
	float delta = angleEx / 8;
	AngleRes.push_back(angleEx);
	for (int i = 0; i < 8; i++)
	{
		AngleRes.push_back(angleEx - delta*(i+1));
	}

	for (int i = 0; i < 8; i++)
	{
		AngleRes.push_back(angleEx + delta * (i + 1));
	}
}

float BoardGraphicsItem::FindAngle(int _angle)
{
	vector<float> temp;
	for (int i = 0; i < Data.size(); i++)
	{
		if (Data[i].Angle() == _angle)
		{
			temp.push_back(Data[i].getRes());
		}
	}
	float sum = 0;
	for (int i = 0; i < temp.size(); i++)
	{
		sum += temp[i];
	}
	return sum/temp.size();
}

vector<DataAnalyze> BoardGraphicsItem::Average()
{
	vector<DataAnalyze> temp;
	int* Angle = VectorInMass(false);
	int* Min = VectorInMass(true);
	float* V2 = VectorInMassV();

	for (int i = 0; i < Data.size(); i++)
	{
		int currentMin = Min[i];
		int currentAngle = Angle[i];
		if (Contains(temp, currentAngle, currentMin))continue;
		float sum = 0;
		int k = 0;
		for (int j = i; j < Data.size(); j++)
		{
			if (currentAngle == Angle[j] && currentMin == Min[j])
			{
				sum += V2[j];
				k++;
			}
		}
		sum /= k;
		DataAnalyze dt;
		dt.setAngleInt(currentAngle);
		dt.setMin(currentMin);
		dt.setRes(sum);
		temp.push_back(dt);
	}
	return temp;
	return vector<DataAnalyze>();
}

int* BoardGraphicsItem::VectorInMass(bool mins)
{
	int* temp = new int[Data.size()];
	for (int i = 0; i < Data.size(); i++)
	{
		if (mins)
			temp[i] = Data[i].getMin();
		else
			temp[i] = Data[i].Angle();
	}
	return temp;
}

float* BoardGraphicsItem::VectorInMassV()
{
	float* V = new float[Data.size()];
	for (int i = 0; i < Data.size(); i++)
	{
		V[i] = Data[i].getRes();
	}
	return V;
}

bool BoardGraphicsItem::Contains(vector<DataAnalyze> cont, int _angle, int _min)
{
	for (int i = 0; i < cont.size(); i++)
	{
		if (cont[i].getMin() == _min)
			if (cont[i].Angle() == _angle)
				return true;
	}
	return false;
}

int BoardGraphicsItem::FindMinV()
{
	float min = Data[0].getRes();
	int idx = 0;
	for (int i = 0; i < Data.size(); i++)
	{
		if (Data[i].getRes() < min)
		{
			min = Data[i].getRes();
			idx = i;
		}
	}
	return idx;
}

/// @brief Анализ параметров изделия 45Д-20-2
/// @details На вход метода поступает набор данных полученных в результате измерения, лежащие в диапазоне от 0 до 90 градусов
/// далее исключаются начальные элемменты (как подверженные максимальному искажению) находиться минимальное значение и оно принимается за нулевую точку
/// и происходит сдвиг всего диапазона (чтобы точка с минимальным значением установилась в нулевое положение) по результатм этих действий должны получить
/// диапазон от -40 до 40 градусов после чего для этого диапазона выполняется анализ линейности.
void BoardGraphicsItem::Analyze()
{
	constexpr std::size_t shift{ 100U };
	bool                  result{ true };
	std::stringstream     ss;
	auto trim = [](Container_t& cont) { std::erase_if(cont, [](const auto& item) { return item > 90.f || item < 0.f; }); };
	trim(*plotV1);
	trim(*plotVPhase1);
	trim(*plotV2);
	trim(*plotVPhase2);
	trim(*plotI1);
	trim(*plotIPhase1);

	//std::erase_if(plV1, [](const auto& item) { return item.x() > 90.f || item.x() < 0.f || item.y() > 90.f || item.y() < 0.f; });
	/*std::erase_if(plV2, [](const auto& item) { return item.x() > 90.f || item.x() < 0.f || item.y() > 90.f || item.y() < 0.f; });
	std::erase_if(plVP1, [](const auto& item) { return item.x() > 90.f || item.x() < 0.f || item.y() > 90.f || item.y() < 0.f; });
	std::erase_if(plVP2, [](const auto& item) { return item.x() > 90.f || item.x() < 0.f || item.y() > 90.f || item.y() < 0.f; });*/

	
	std::vector<Point_t> v1{ plV1.size() };
	std::vector<Point_t> vPhase1{ plVP1.size() };
	std::vector<Point_t> v2{ plV2.size() };
	std::vector<Point_t> vPhase2{ plVP2.size() };

	std::ranges::copy(plV1.begin(), plV1.end(), v1.begin());
	std::ranges::copy(plVP1.begin(), plVP1.end(), vPhase1.begin());
	std::ranges::copy(plV2.begin(), plV2.end(), v2.begin());
	std::ranges::copy(plVP2.begin(), plVP2.end(), vPhase2.begin());

	QMessageBox msgBox;

	{
		auto [min_el, max_el] = std::minmax_element(v1.begin(), v1.end(), [](Point<float> a, Point<float> b) { return a.x() < b.x(); });
		msgBox.setText(QString::fromStdString(to_string(min_el->x()) + " " + to_string(min_el->y()) + "  >>> "
			+ to_string(max_el->x()) + "-" + to_string(max_el->y())));
		msgBox.exec();
	}

	/*std::vector<Point_t> v1 = EraseForAngle(tempv1,-360,360);
	std::vector<Point_t> vPhase1 = EraseForAngle(tempvPhase1, -360, 360);
	std::vector<Point_t> v2 = EraseForAngle(tempv2, -360, 360);
	std::vector<Point_t> vPhase2 = EraseForAngle(tempvPhase2, -360, 360);*/
	


	auto filter = std::bind(DSP::median, std::placeholders::_1, std::placeholders::_2, [](const Point_t& p1, const Point_t& p2) { return p1.y() < p2.y(); });
	/// Применяем цифровой фильтр
	v1 = DSP::smooth<5>(v1, filter);
	vPhase1 = DSP::smooth<5>(vPhase1, filter);
	v2 = DSP::smooth<5>(v2, filter);
	vPhase2 = DSP::smooth<5>(vPhase2, filter);

	/*tempv1 = EraseForAngle(v1, -40, 40);
	tempvPhase1 = EraseForAngle(vPhase1, -40, 40);
	tempv2 = EraseForAngle(v2, -40, 40);
	tempvPhase2 = EraseForAngle(vPhase2, -40, 40);

	std::ranges::copy(tempv1.begin(), tempv1.end(), v1.begin());
	std::ranges::copy(tempvPhase1.begin(), tempvPhase1.end(), vPhase1.begin());
	std::ranges::copy(tempv2.begin(), tempv2.end(), v2.begin());
	std::ranges::copy(tempvPhase2.begin(), tempvPhase2.end(), vPhase2.begin());*/

	{
		auto [min_el, max_el] = std::minmax_element(v1.begin(), v1.end(), [](Point<float> a, Point<float> b) { return a.x() < b.x(); });
		msgBox.setText(QString::fromStdString(to_string(min_el->x()) + " " + to_string(min_el->y()) + "  >>> "
			+ to_string(max_el->x()) + "-" + to_string(max_el->y())));
		msgBox.exec();
	}


	auto min = *std::min_element(v1.begin(), v1.end(), [](const Point_t& a, const Point_t& b) { return a.y() < b.y(); });


	ss << info.getMeter().toStdString() << "\t" << info.getDocument().toStdString() << "\t" << info.getNumber().toStdString() << "\t" << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	
	(*tableResult)(3, 0) = std::make_pair(QString("Umin\n3.1.6"), Qt::white);
	ss << "\t" << min.y();
	(*tableResult)(3, 1) = std::make_pair(QString("%1 mV").arg(min.y(), 0, 'f', 3), Qt::white);
	if (min.y() > 2.5f) result = false;

	auto itV1 = v1.begin();
	auto itVPhase1 = vPhase1.begin();
	auto itV2 = v2.begin();
	auto itVPhase2 = vPhase2.begin();

	using elem_t = std::tuple<float, float, float, float, float>;
	std::vector<elem_t> points;
	points.reserve(v1.size());
	std::size_t idx{ 0 };
	while (itV1 != v1.end() || itVPhase1 != vPhase1.end() || itV2 != v2.end() || itVPhase2 != vPhase2.end()) 
	{
		
		points.emplace_back(itV1->x() - min.x(), itV1->y(), itVPhase1->y(), itV2->y(), itVPhase2->y());
		++itV1;
		++itVPhase1;
		++itV2;
		++itVPhase2;
	}
	auto [min_el, max_el] = std::minmax_element(points.begin(), points.end(), [](const elem_t& a, const elem_t& b) { return std::get<0>(a) < std::get<0>(b); });
	msgBox.setText(QString::fromStdString(to_string(std::get<0>(*min_el)) + " " + to_string(std::get<0>(*max_el))));
	msgBox.exec();

	if (std::get<0>(*min_el) < -40.f && std::get<0>(*max_el) > 40.f)
	{
		
		float  err{ 1 };
		elem_t start;
		std::for_each(points.begin(), points.end(), [&start, &err](const elem_t& el)
		{
			if (std::abs(std::get<0>(el) - 40.f) < err)
			{
				err = std::abs(std::get<0>(el) - 40.f);
				start = el;
			}
		});

		//Заполнение грaдyсов в таблице
		const auto                   coeff = 4000.f / std::get<1>(start);
		std::initializer_list<float> ctrls{ 4000, 3500, 3000, 2500, 2000, 1500, 1000, 500, -500, -1000, -1500, -2000, -2500, -3000, -3500, -4000 };
		std::set<Point_t>            res;
		for (auto it = ctrls.begin(); it != ctrls.end(); ++it) 
		{
			std::map<float, elem_t> err_map;
			for (const auto& item : points) 
			{
				const auto sign = std::cos(std::get<2>(item)) > 0 ? 1 : -1;
				err_map.emplace(std::abs(std::get<1>(item) * coeff * sign - *it), item);
			}

			(*tableResult)(5, 0) = std::make_pair(QString("I\n3.1.6"), Qt::white);
			ss << "\t" << bufI1.apply(DSP::mean);
			if (bufI1.apply(DSP::mean) > 6.f) 
			{
				(*tableResult)(5, 1) = std::make_pair(QString("%1 mA").arg(bufI1.apply(DSP::mean), 0, 'f', 3), Qt::red);
				result = false;
			}
			else 
			{
				(*tableResult)(5, 1) = std::make_pair(QString("%1 mA").arg(bufI1.apply(DSP::mean), 0, 'f', 3), Qt::white);
			}

			auto min_err = std::min_element(err_map.begin(), err_map.end());
			if (min_err != err_map.end()) 
			{
				const auto point{ min_err->second };
				/// Определяем коэф. трансформации и выходное напряжение только в положении 40 градусов
				if (it == ctrls.begin()) 
				{
					/// Определяем выходное напряжение
					(*tableResult)(4, 0) = std::make_pair(QString("Ui\n3.1.6"), Qt::white);
					ss << "\t" << std::get<1>(point);
					if (std::get<1>(point) > 3300.f || std::get<1>(point) < 2700.f) 
					{
						(*tableResult)(4, 1) = std::make_pair(QString("%1 mV").arg(std::get<1>(point), 0, 'f', 3), Qt::red);
						result = false;
					}
					else 
					{
						(*tableResult)(4, 1) = std::make_pair(QString("%1 mV").arg(std::get<1>(point), 0, 'f', 3), Qt::white);
					}
					/// Расчитываем коэф. трансформации
					(*tableResult)(7, 0) = std::make_pair(QString("Ucomp/Ui\n3.1.8"), Qt::white);
					const auto trans{ std::get<3>(point) / std::get<1>(point)};
					ss << "\t" << trans;
					(*tableResult)(8, 0) = std::make_pair(QString("Class"), Qt::white);
					if (trans <= 0.0485f && trans >= 0.0481f) 
					{
						(*tableResult)(7, 1) = std::make_pair(QString("%1").arg(trans, 0, 'f', 4), Qt::white);
						(*tableResult)(8, 1) = std::make_pair(QString("1"), Qt::white);
						ss << "\t1";
					}
					else if (trans <= 0.0487f && trans >= 0.0479f) 
					{
						(*tableResult)(7, 1) = std::make_pair(QString("%1").arg(trans, 0, 'f', 4), Qt::green);
						(*tableResult)(8, 1) = std::make_pair(QString("2"), Qt::white);
						ss << "\t2";
					}
					else if (trans <= 0.0492f && trans >= 0.0474f) 
					{
						(*tableResult)(7, 1) = std::make_pair(QString("%1").arg(trans, 0, 'f', 4), Qt::yellow);
						(*tableResult)(8, 1) = std::make_pair(QString("3"), Qt::white);
						ss << "\t3";
					}
					else 
					{
						(*tableResult)(7, 1) = std::make_pair(QString("%1").arg(trans, 0, 'f', 4), Qt::red);
						(*tableResult)(8, 1) = std::make_pair(QString("No relevant."), Qt::red);
						ss << "\t-";
						result = false;
					}
				}
				/// Определяем погрешность линейности в заданных точках
				(*tableError)(it - ctrls.begin(), 0) = std::make_pair(QString("%1˚").arg(*it / 100), Qt::white);
				float it_error;
				if (*it > 0) 
				{
					it_error = std::get<0>(point) - *it / 100;
				}
				else 
				{
					it_error = *it / 100 - std::get<0>(point);
					//std::tie(it_error, std::ignore) = res.emplace(*it / 100, *it / 100 - std::get<0>(point));
				}
				(*tableError)(it - ctrls.begin(), 1) = std::make_pair(QString("%1").arg(convert(it_error)), Qt::white);
				ss << "\t" << it_error;
			}
		}
		

		auto [tol_min, tol_max] = std::minmax_element(res.begin(), res.end(), [](const Point_t &a, const Point_t &b) { return a.y() < b.y(); });
		(*tableResult)(6, 0)    = std::make_pair(QString("Errors\n3.1.7"), Qt::white);
		const auto error        = std::abs((tol_max->y() - tol_min->y()) / 2.f) * 60.f / 24.f;
		ss << "\t" << error;
		if (error >= 0.7f) {
			(*tableResult)(6, 1) = std::make_pair(QString("\u00B1%1\%").arg(error, 0, 'f', 3), Qt::red);
			result               = false;
		}
		else {
			(*tableResult)(6, 1) = std::make_pair(QString("\u00B1%1\%").arg(error, 0, 'f', 3), Qt::white);
		}
		(*tableResult)(9, 0) = std::make_pair(QString("Result"), Qt::white);
		if (result) {
			(*tableResult)(9, 1) = std::make_pair(QString("Good"), Qt::green);
			ss << "\tГоден";
		}
		else {
			(*tableResult)(9, 1) = std::make_pair(QString("Not good"), Qt::red);
			ss << "\tНе годен";
		}
		ss << "\n";
		update();
		std::ofstream file("data.txt", std::ios_base::out | std::ios_base::app);
		if (file) {
			file << ss.str();
		}
	}
	else 
	{
		tableResult->clear();
		(*tableResult)(9, 0) = std::make_pair(QString("Not currently"), Qt::yellow);
		(*tableResult)(9, 1) = std::make_pair(QString("installed"), Qt::yellow);
	}
}
void BoardGraphicsItem::Offset()
{
	int indMin = FindMinV();
	for (int i = 0; i < Data.size(); i++)
	{
		Data[i].setAngle(Data[i].getAngle() - Data[indMin].getAngle());
	}
}
;