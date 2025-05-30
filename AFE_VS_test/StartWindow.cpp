#include "StartWindow.hpp"

StartWindow::StartWindow(QWidget* parent)
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

	//Main window ================================================================================
	QString title = "Meter ";
	window->setWindowTitle(title);
	CreateWindow();
	window->show();

	qApp->setPalette(darkPalette);
	QApplication::exec();
}

int StartWindow::BtnSkt232()
{
	StepWindow *s; // 45Д20-2; СКТ-232Б; СКТ-265Д
	QCoreApplication::setApplicationName("Метер");
	s = new StepWindow("СКТ-232Б");
	window->close();
	return 0;
}

int StartWindow::Btn45D20()
{
	StepWindow *s; // 45Д20-2; СКТ-232Б; СКТ-265Д
	QCoreApplication::setApplicationName("Метер");
	s = new StepWindow("45Д20-2");
	window->close();
	return 0;
}

int StartWindow::BtnSkt265()
{
	StepWindow *s; // 45Д20-2; СКТ-232Б; СКТ-265Д
	QCoreApplication::setApplicationName("Метер");
	s = new StepWindow("СКТ-265Д");
	window->close();
	return 0;
}

void StartWindow::CreateWindow()
{
	btnSkt232 = new QPushButton("Метер СКТ-232Б");
	btnSkt232->setMinimumWidth(200);
	btnSkt232->setMinimumHeight(50);
	btnSkt232->setStyleSheet("QPushButton{"
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
	btnSkt265 = new QPushButton("Метер СКТ-265Д");
	btnSkt265->setMinimumWidth(200);
	btnSkt265->setMinimumHeight(50);
	btnSkt265->setStyleSheet("QPushButton{"
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
	btn45D20 = new QPushButton("Метер 45Д-20-2");
	btn45D20->setMinimumWidth(200);
	btn45D20->setMinimumHeight(50);
	btn45D20->setStyleSheet("QPushButton{"
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

	QObject::connect(btnSkt232, &QPushButton::clicked, this, &StartWindow::BtnSkt232);
	QObject::connect(btn45D20, &QPushButton::clicked, this, &StartWindow::Btn45D20);
	QObject::connect(btnSkt265, &QPushButton::clicked, this, &StartWindow::BtnSkt265);

	QVBoxLayout* main = new QVBoxLayout();
	//main->addWidget(btnMonitor);
	main->addWidget(btn45D20);
	main->addWidget(btnSkt232);
	main->addWidget(btnSkt265);
	window->setLayout(main);
}

StartWindow::~StartWindow()
{

}
