#pragma once

#include <QGraphicsView>
#include <QtWidgets>
#include <QObject>
#include <QtSerialPort/QSerialPort>
#include <math.h>
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <string_view>
#include <vector>

class DataAnalyze
{
private :
	float angle;
	float res;
public:
	DataAnalyze(float value1, float value2) { angle = value1; res = value2; }
	float getAngle() const { return angle; }
	float getRes() const { return res; }
	void setAngle(float value) { angle = value; }
	void setRes(float value) { res = value; }

};