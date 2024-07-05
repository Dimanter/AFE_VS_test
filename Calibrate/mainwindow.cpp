#include "mainwindow.h"

//возвращает установленную частоту 
float MainWindow::Frequency() const
{
    return ui->Frequency->value();
}

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::mainwindow)
{
    ui->setupUi(this);//открывает интерфейс
    this->setFixedSize(780, 480);//устанавливает фиксированный размер окна
    
    OutP->raw.resize(N);//выделение памяти
    OutN->raw.resize(N);//выделение памяти

    dispatcher.registerTransport(transport);//регистрирует транспортный уровень

    dispatcher.registerService(coefsOut);//регестрирует сервис коэфицентов выхода
    dispatcher.registerService(coefsIn1);//регестрирует сервис коэфицентов 1 входа
    dispatcher.registerService(coefsIn2);//регестрирует сервис коэфицентов 2 входа
    dispatcher.registerService(out);//регестрирует сервис вывода информации
    dispatcher.registerService(control);//регестрирует сервис контроля команд
    dispatcher.registerService(parameter);//регестрирует сервис параметров
    dispatcher.registerService(message);//регестрирует сервис эвентов
    dispatcher.registerService(measure);//регестрирует сервис измерений
    dispatcher.registerService(OutP);//регестрирует сервис чистых данных
    dispatcher.registerService(OutN);//регестрирует сервис чистых данных
    //добавить процесс сохранения коэфицентов в контроллер
    processes.add(processSave, sendCoefOut());
    processes.add(processSave, sendCoefIn1());
    processes.add(processSave, sendCoefIn2());
    processes.add(processSave, save());
    //добавить процесс загрузки коэфицентов из контроллера
    processes.add(processLoad, load());
    processes.add(processLoad, receiveCoefOut());
    processes.add(processLoad, receiveCoefIn1());
    processes.add(processLoad, receiveCoefIn2());
    //добавить процесс усиления выхода
    addProcessOut(processOut_x1, coefOutService::gain::x1, 2000, 3000);
    addProcessOut(processOut_x2, coefOutService::gain::x2, 2000, 3000);
    addProcessOut(processOut_x4, coefOutService::gain::x4, 2000, 3000);
    addProcessOut(processOut_x8, coefOutService::gain::x8, 2000, 2650);
    //добавить процесс усиления входа
    addProcessIn(processIn_x1_8, coefInService::gain::x1_8, coefOutService::gain::x4, coefOutService::gain::x8, 4000, 2650);
    addProcessIn(processIn_x1_4, coefInService::gain::x1_4, coefOutService::gain::x2, coefOutService::gain::x4, 4000, 4000);
    addProcessIn(processIn_x1_2, coefInService::gain::x1_2, coefOutService::gain::x1, coefOutService::gain::x2, 4000, 4000);
    addProcessIn(processIn_x001, coefInService::gain::x001, coefOutService::gain::x1, coefOutService::gain::x1, 1000, 4000);
    /// С использованием делителя 1:16
    addProcessIn(processIn_x002, coefInService::gain::x002, coefOutService::gain::x4, coefOutService::gain::x8, 4000, 2650);
    addProcessIn(processIn_x004, coefInService::gain::x004, coefOutService::gain::x2, coefOutService::gain::x4, 4000, 4000);
    addProcessIn(processIn_x008, coefInService::gain::x008, coefOutService::gain::x1, coefOutService::gain::x2, 4000, 4000);
    addProcessIn(processIn_x016, coefInService::gain::x016, coefOutService::gain::x1, coefOutService::gain::x1, 1000, 4000);
    /// С использованием делителя 1:192
    addProcessIn(processIn_x032, coefInService::gain::x032, coefOutService::gain::x4, coefOutService::gain::x8, 4000, 2650);
    addProcessIn(processIn_x064, coefInService::gain::x064, coefOutService::gain::x2, coefOutService::gain::x4, 2500, 2500);
    addProcessIn(processIn_x128, coefInService::gain::x128, coefOutService::gain::x1, coefOutService::gain::x2, 2500, 2500);

    ui->btnDisconnect->setDisabled(true);//отключить кнопку дисконект
    const auto serialPortInfos = QSerialPortInfo::availablePorts();//найти доступные порты
    for (const QSerialPortInfo& portInfo : serialPortInfos) {
        ui->lstPort->addItem(portInfo.portName(), portInfo.productIdentifier());//добавить порты в раскрывающийся список
    }
    //добавление элементов в раскрывающийся список выходного усиления
    ui->lstOutGain->addItem("x1", processOut_x1);
    ui->lstOutGain->addItem("x2", processOut_x2);
    ui->lstOutGain->addItem("x4", processOut_x4);
    ui->lstOutGain->addItem("x8", processOut_x8);
    //добавление элементов в раскрывающийся список входного усиления
    ui->lstInGain->addItem("x1/8", processIn_x1_8);
    ui->lstInGain->addItem("x1/4", processIn_x1_4);
    ui->lstInGain->addItem("x1/2", processIn_x1_2);
    ui->lstInGain->addItem("x1", processIn_x001);
    ui->lstInGain->addItem("x2", processIn_x002);
    ui->lstInGain->addItem("x4", processIn_x004);
    ui->lstInGain->addItem("x8", processIn_x008);
    ui->lstInGain->addItem("x16", processIn_x016);
    ui->lstInGain->addItem("x32", processIn_x032);
    ui->lstInGain->addItem("x64", processIn_x064);
    ui->lstInGain->addItem("x128", processIn_x128);

    port = new QSerialPort(this);//параметр порта
    connect(port, &QSerialPort::errorOccurred, this, &MainWindow::portError);//соединение порта с отловом ошибок
}

MainWindow::~MainWindow()
{}

void MainWindow::eventProcess(const Processes::processID_t& service)
{
}

void MainWindow::notify(const Service::serviceID_t& service, const Service::State& st)
{
    processes.notify(service, st);
    switch (static_cast<Services>(service)) 
    {
    case Services::CoefsOut://выходной коэфицент
        switch (st)
        {
        case Service::State::ReceiveComplete://успешно получен
            ui->textLog->appendPlainText("The output coefficients are received.\n");
            break;
        case Service::State::TransmitComplete://успешно передан
            ui->textLog->appendPlainText("The output coefficients have been transmitted.\n");
            break;
        }
        break;
    case Services::CoefsIn1://коэфицент входа 1 канала
        switch (st) 
        {
        case Service::State::ReceiveComplete://успешно получен
            ui->textLog->appendPlainText("The acquisition of the input coefficient on channel 1 is completed.\n");
            break;
        case Service::State::TransmitComplete://успешно передан
            ui->textLog->appendPlainText("The input coefficients were transmitted via channel 1.\n");
            break;
        }
        break;
    case Services::CoefsIn2://коэфицент входа 2 канала
        switch (st) 
        {
        case Service::State::ReceiveComplete://успешно получен
            ui->textLog->appendPlainText("The acquisition of the input coefficient on channel 2 is completed.\n");
            break;
        case Service::State::TransmitComplete://успешно передан
            ui->textLog->appendPlainText("The input coefficients were transmitted via channel 2\n");
            break;
        }
        break;
    case Services::Parameters://считывание параметров с 1 и 2 канала
        in1 = (*measure->Result).V1.RMS;//напряжение 1 канала
        in2 = (*measure->Result).V2.RMS;//напряжение 2 канала
        phase1 = (*measure->Result).V1.Phase;//фаза 1 канала
        phase2 = (*measure->Result).V2.Phase;//фаза 2 канала
        break;
    }
}

void MainWindow::readyRead()
{
    auto data = port->readAll();//считать всё
    transport->receive(reinterpret_cast<unsigned char*>(data.data()), data.size());//передать на транспортный уровень
}

void MainWindow::portError(QSerialPort::SerialPortError error)
{
    switch (error)
    {
    case QSerialPort::ResourceError://ошибка данных
        port->close();//отключаем порт
        ui->btnConnect->setDisabled(false);//включить кнопку конект
        ui->btnDisconnect->setDisabled(true);//отключить кнопку дисконект
        break;
    default:
        break;
    }
}

uint8_t MainWindow::Write(std::unique_ptr<addressing_t[]> data, std::size_t size)
{
    return port->write(reinterpret_cast<const char*>(data.get()), size);//запись в порт
}


void MainWindow::on_btnRefresh_clicked()
{
    ui->lstPort->clear();//очистить список портов 
    const auto serialPortInfos = QSerialPortInfo::availablePorts();//все доступные порты
    for (const QSerialPortInfo& portInfo : serialPortInfos) 
    {
        ui->lstPort->addItem(portInfo.portName(), portInfo.productIdentifier());//добавить элементы в раскрывающийся список портов
    }
}

void MainWindow::on_btnConnect_clicked()
{
    try 
    {
        port->setPortName(ui->lstPort->currentText());//установить выбраное имя порта 
        //настройки порта
        if (port->setBaudRate(7372800) && 
            port->setDataBits(QSerialPort::Data8) && 
            port->setParity(QSerialPort::NoParity) && 
            port->setStopBits(QSerialPort::OneStop) && 
            port->setFlowControl(QSerialPort::HardwareControl)) 
        {
            if (port->open(QIODevice::ReadWrite))//открыть для чтения и записи 
            {
                if (port->isOpen())//если открыт
                {
                    dispatcher.resetService();//сбросить сервисы
                    qDebug() << (port->portName() + " >> Open!");
                }
                else {
                    port->close();//закрыть порт
                    qDebug() << port->errorString().toLocal8Bit();//вывести ошибку
                }
            }
        }

        connect(port, &QSerialPort::readyRead, this, &MainWindow::readyRead);//соединение чтения порта с функцие чтения

        ui->btnConnect->setDisabled(true);//отключить кнопку конект
        ui->btnDisconnect->setDisabled(false);//включить кнопку дисконект
    }
    catch (...) 
    {
        qDebug("Do not create serial port object");
    }
}

void MainWindow::on_btnDisconnect_clicked()
{
    if (port->isOpen())//если порт открыт
    {
        qDebug() << (port->portName() + " >> Close!");
        port->close();//закрыть порт
    }
    ui->btnConnect->setDisabled(false);//отключить кнопку конект
    ui->btnDisconnect->setDisabled(true);//отключить кнопку дисконект
}

void MainWindow::showSetting(void)//не работает
{
    if (out->exec() == QDialog::Accepted) 
    {
        if (port->isOpen()) 
        {
            out->transmit(1, Service::Type::TransmitStreaming);
        }
        else {
            ui->textLog->appendPlainText("Port is not open!\n");
        }
    }
}

void MainWindow::on_btnStart_clicked()
{
    if (port->isOpen())//если порт открыт
    {
        switch (ui->tabWidget->currentIndex())//переключение по вкладкам 0 - выход, 1 - вход
        {
        case 0: ///< Калибровка выхода
            if (ui->rbOutMin->isChecked()) 
            {
                processes.run(ui->lstOutGain->currentData().toUInt() | flagMin);//запустить процесс калибровки в минимуме
            }
            else 
            {
                processes.run(ui->lstOutGain->currentData().toUInt());//запустить процесс калибровки
            }
            break;
        case 1: ///< Калибровка входа
            if (ui->rbIn1->isChecked()) 
            {
                if (ui->rbInMin->isChecked()) 
                {
                    processes.run(ui->lstInGain->currentData().toUInt() | flagInCh1 | flagMin);//запустить процесс калибровки в минимуме на 1 канале
                }
                else {
                    processes.run(ui->lstInGain->currentData().toUInt() | flagInCh1);//запустить процесс калибровки на 1 канале
                }
            }
            else 
            {
                if (ui->rbInMin->isChecked()) 
                {
                    processes.run(ui->lstInGain->currentData().toUInt() | flagMin);//запустить процесс калибровки в минимуме на 2 канале
                }
                else 
                {
                    processes.run(ui->lstInGain->currentData().toUInt());//запустить процесс калибровки на 2 канале
                }
            }
            break;
        }
    }
    else 
    {
        ui->textLog->appendPlainText("Port is not open!\n");
    }

}

void MainWindow::on_btnStop_clicked()
{
    if (port->isOpen()) 
    {
        control->command = controlService::Stop;//стоп всех процессов контроллера
        control->transmit(1, Service::Type::TransmitStreaming);
    }
    else {
        ui->textLog->appendPlainText("Port is not open!\n");
    }
}

void MainWindow::on_btnSave_clicked()
{
    if (port->isOpen()) {
        processes.run(processSave);
    }
    else {
        ui->textLog->appendPlainText("Port is not open!\n");
    }
}

void MainWindow::on_btnLoad_clicked()
{
    if (port->isOpen()) {
        processes.run(processLoad);
    }
    else {
        ui->textLog->appendPlainText("Port is not open!\n");
    }
}

void MainWindow::on_lstOutGain_currentIndexChanged(int index)
{
    ui->OutMin->setValue(0.f);//сбросить строку минимума на вкладке выход
    ui->OutMax->setValue(0.f);//сбросить строку максимума на вкладке выход
}

void MainWindow::on_btnCoefIn1_clicked()
{
    if (coefsIn1->exec() == QDialog::Accepted) {
        coefsIn1->updateData();
    }
}

void MainWindow::on_btnCoefIn2_clicked()
{
    if (coefsIn2->exec() == QDialog::Accepted) {
        coefsIn2->updateData();
    }
}

void MainWindow::on_lstInGain_currentIndexChanged(int index)
{
    ui->inMin->setValue(0.f);//сбросить строку минимума на вкладке вход
    ui->inMax->setValue(0.f);//сбросить строку максимума на вкладке вход
}

void MainWindow::on_btnFixIn_clicked()
{
    if (ui->rbInMin->isChecked()) {
        if (ui->rbIn1->isChecked()) {
            in1_min = in1;//записать минимум 1 канала
        }
        else {
            in2_min = in2;//записать минимум 2 канала
        }
        ui->inMin->setValue(ui->inCal->value());
    }
    else {
        if (ui->rbIn1->isChecked()) {
            in1_max = in1;//записать максимум 1 канала
        }
        else {
            in2_max = in2;//записать максимум 2 канала
        }
        ui->inMax->setValue(ui->inCal->value());
    }
}

void MainWindow::on_btnCalIn_clicked()
{
    if (ui->rbIn1->isChecked()) {
        calIn(coefsIn1, '1', in1_min, in1_max, phase1);//расчитать коэфиценты для 1 канала
    }
    else {
        calIn(coefsIn2, '2', in2_min, in2_max, phase2);//расчитать коэфиценты для 2 канала
    }
}

void MainWindow::calIn(std::shared_ptr<coefInService> coefsIn, char channel, float min, float max, float phase)
{
    switch (ui->lstInGain->currentData().toUInt()) {
    case processIn_x1_8://1:8
        coefsIn->Volt_1_8_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_1_8_b = ui->inMin->value() - min * coefsIn->Volt_1_8_a;//расчитать коэфицент б
        coefsIn->Phase_1_8 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1/8: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_1_8_a).arg(coefsIn->Volt_1_8_b).arg(coefsIn->Phase_1_8));
        break;
    case processIn_x1_4://1:4
        coefsIn->Volt_1_4_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_1_4_b = ui->inMin->value() - min * coefsIn->Volt_1_4_a;//расчитать коэфицент б
        coefsIn->Phase_1_4 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1/4: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_1_4_a).arg(coefsIn->Volt_1_4_b).arg(coefsIn->Phase_1_4));
        break;
    case processIn_x1_2://1:2
        coefsIn->Volt_1_2_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_1_2_b = ui->inMin->value() - min * coefsIn->Volt_1_2_a;//расчитать коэфицент б
        coefsIn->Phase_1_2 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1/2: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_1_2_a).arg(coefsIn->Volt_1_2_b).arg(coefsIn->Phase_1_2));
        break;
    case processIn_x001://1:1
        coefsIn->Volt_001_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_001_b = ui->inMin->value() - min * coefsIn->Volt_001_a;//расчитать коэфицент б
        coefsIn->Phase_001 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_001_a).arg(coefsIn->Volt_001_b).arg(coefsIn->Phase_001));
        break;
    case processIn_x002://х2
        coefsIn->Volt_002_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_002_b = ui->inMin->value() - min * coefsIn->Volt_002_a;//расчитать коэфицент б
        coefsIn->Phase_002 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x2: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_002_a).arg(coefsIn->Volt_002_b).arg(coefsIn->Phase_002));
        break;
    case processIn_x004://х4
        coefsIn->Volt_004_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_004_b = ui->inMin->value() - min * coefsIn->Volt_004_a;//расчитать коэфицент б
        coefsIn->Phase_004 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x4: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_004_a).arg(coefsIn->Volt_004_b).arg(coefsIn->Phase_004));
        break;
    case processIn_x008://х8
        coefsIn->Volt_008_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_008_b = ui->inMin->value() - min * coefsIn->Volt_008_a;//расчитать коэфицент б
        coefsIn->Phase_008 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x8: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_008_a).arg(coefsIn->Volt_008_b).arg(coefsIn->Phase_008));
        break;
    case processIn_x016://х16
        coefsIn->Volt_016_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_016_b = ui->inMin->value() - min * coefsIn->Volt_016_a;//расчитать коэфицент б
        coefsIn->Phase_016 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x16: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_016_a).arg(coefsIn->Volt_016_b).arg(coefsIn->Phase_016));
        break;
    case processIn_x032://х32
        coefsIn->Volt_032_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_032_b = ui->inMin->value() - min * coefsIn->Volt_032_a;//расчитать коэфицент б
        coefsIn->Phase_032 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x32: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_032_a).arg(coefsIn->Volt_032_b).arg(coefsIn->Phase_032));
        break;
    case processIn_x064://х64
        coefsIn->Volt_064_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_064_b = ui->inMin->value() - min * coefsIn->Volt_064_a;//расчитать коэфицент б
        coefsIn->Phase_064 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x64: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_064_a).arg(coefsIn->Volt_064_b).arg(coefsIn->Phase_064));
        break;
    case processIn_x128://х128
        coefsIn->Volt_128_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);//расчитать коэфицент а
        coefsIn->Volt_128_b = ui->inMin->value() - min * coefsIn->Volt_128_a;//расчитать коэфицент б
        coefsIn->Phase_128 = phase - ui->inCalPhase->value();//расчитать фазу
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x128: a=%2; b=%3; phase=%4\n").arg(channel).arg(coefsIn->Volt_128_a).arg(coefsIn->Volt_128_b).arg(coefsIn->Phase_128));
        break;
    }
}

void MainWindow::on_btnCoefOut_clicked()
{
    if (coefsOut->exec() == QDialog::Accepted) {
        coefsOut->updateData();//обновить коэфиценты
    }
}

void MainWindow::on_btnFixOut_clicked()
{
    if (ui->rbOutMin->isChecked()) {
        ui->OutMin->setValue(ui->outCal->value());//зафиксировать минимум
    }
    else {
        ui->OutMax->setValue(ui->outCal->value());//зафиксировать максимум
    }
}

void MainWindow::on_btnCalOut_clicked()
{
    switch (ui->lstOutGain->currentData().toUInt()) {
    case processOut_x1://х1
        coefsOut->Out_x1_a = (ui->OutMax->value() - ui->OutMin->value()) / (3000.f - 2000.f);//расчитать коэфицент а
        coefsOut->Out_x1_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x1_a;//расчитать коэфицент б
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain x1: a=%1; b=%2;\n").arg(coefsOut->Out_x1_a).arg(coefsOut->Out_x1_b));
        break;
    case processOut_x2://х2
        coefsOut->Out_x2_a = (ui->OutMax->value() - ui->OutMin->value()) / (3000.f - 2000.f);//расчитать коэфицент а
        coefsOut->Out_x2_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x2_a;//расчитать коэфицент б
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain. x2: a=%1; b=%2;\n").arg(coefsOut->Out_x2_a).arg(coefsOut->Out_x2_b));
        break;
    case processOut_x4://х4
        coefsOut->Out_x4_a = (ui->OutMax->value() - ui->OutMin->value()) / (3000.f - 2000.f);//расчитать коэфицент а
        coefsOut->Out_x4_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x4_a;//расчитать коэфицент б
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain. x4: a=%1; b=%2;\n").arg(coefsOut->Out_x4_a).arg(coefsOut->Out_x4_b));
        break;
    case processOut_x8://х8
        coefsOut->Out_x8_a = (ui->OutMax->value() - ui->OutMin->value()) / (2650.f - 2000.f);//расчитать коэфицент а
        coefsOut->Out_x8_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x8_a;//расчитать коэфицент б
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain. x8: a=%1; b=%2;\n").arg(coefsOut->Out_x8_a).arg(coefsOut->Out_x8_b));
        break;
    }
}

void MainWindow::on_textLog_textChanged()
{
    QTextCursor cursor = ui->textLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(cursor);
}
