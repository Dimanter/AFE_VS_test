#pragma once

#include <QDialog>

namespace Ui 
{	
	class device;
}
//@brief Device Класс данных подключенного оборудования
class device : public QDialog
{
	Q_OBJECT

public :
	explicit device(QWidget* parent = nullptr);
	QString getMeter();
	QString getDocument();
	QString getNumber();
	~device();
private :
	Ui::device* ui;
};