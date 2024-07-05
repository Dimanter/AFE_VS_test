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
	float angleGrad;// Угол в градусах с плавающей запятой
	int angle;// Угол в градусах целочисленный
	int min;// Минуты угла
	int sec;// Секунды угла
	float V1;// Напряжение на первом канале
	float V2;// Напряжение на втором канале
	float Phase1;// Фаза первого канала
	float Phase2;// фаза второго канала
	float I;// Сила тока
	float IPhase;// Фаза тока
public:
	DataAnalyze(float angle, float Rms1,float Rms2,float P1,float P2, float IA, float IP) 
	{ 
		angleGrad = angle;
		V1 = Rms1;
		V2 = Rms2;
		Phase1 = P1;
		Phase2 = P2;
		I = IA;
		IPhase = IP;
		convertAngle();
	}
	//конвертация угла в минуты, секунды 
	void convertAngle()
	{
		angle = std::trunc(angleGrad);
		min = angleGrad - angle;
		min = std::trunc(min * 60.0);
		sec = (angleGrad - angle) * 60.0 - min;
		sec = std::trunc(sec * 60.0);
	}
	//возвраает угол
	float getAngle() const { return angleGrad; }
	//возвращает напряжение
	float getRes() const { return V1; }
	//устанавливает угол
	void setAngle(float value) { angleGrad = value; }
	//устанавливает напряжение
	void setRes(float value) { V1 = value; }

};