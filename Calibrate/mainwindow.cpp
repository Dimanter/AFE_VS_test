
#include "mainwindow.h"

float MainWindow::Frequency() const
{
    return ui->Frequency->value();
}


MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), ui(new Ui::mainwindow)
{
    ui->setupUi(this);
    this->setFixedSize(780, 480);
    
    //OutP->raw.resize(N);
    //OutN->raw.resize(N);

    dispatcher.registerTransport(transport);

    dispatcher.registerService(coefsOut);
    dispatcher.registerService(coefsIn1);
    dispatcher.registerService(coefsIn2);
    dispatcher.registerService(out);
    dispatcher.registerService(control);
    dispatcher.registerService(parameter);
    dispatcher.registerService(message);
    dispatcher.registerService(measure);
    dispatcher.registerService(OutP);
    dispatcher.registerService(OutN);

    processes.add(processSave, sendCoefOut());
    processes.add(processSave, sendCoefIn1());
    processes.add(processSave, sendCoefIn2());
    processes.add(processSave, save());

    processes.add(processLoad, load());
    processes.add(processLoad, receiveCoefOut());
    processes.add(processLoad, receiveCoefIn1());
    processes.add(processLoad, receiveCoefIn2());

    addProcessOut(processOut_x1, coefOutService::gain::x1, 2000, 3000);
    addProcessOut(processOut_x2, coefOutService::gain::x2, 2000, 3000);
    addProcessOut(processOut_x4, coefOutService::gain::x4, 2000, 3000);
    addProcessOut(processOut_x8, coefOutService::gain::x8, 2000, 2650);

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

    ui->btnDisconnect->setDisabled(true);
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : serialPortInfos) {
        ui->lstPort->addItem(portInfo.portName(), portInfo.productIdentifier());
    }

    ui->lstOutGain->addItem("x1", processOut_x1);
    ui->lstOutGain->addItem("x2", processOut_x2);
    ui->lstOutGain->addItem("x4", processOut_x4);
    ui->lstOutGain->addItem("x8", processOut_x8);

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

    port = new QSerialPort(this);
    connect(port, &QSerialPort::errorOccurred, this, &MainWindow::portError);
}

MainWindow::~MainWindow()
{}

void MainWindow::eventProcess(const Processes::processID_t& service)
{
}
//отслеживание работы процессов
void MainWindow::notify(const Service::serviceID_t& service, const Service::State& st)
{
    processes.notify(service, st);
    switch (static_cast<Services>(service)) {
    case Services::CoefsOut:
        switch (st)
        {
        case Service::State::ReceiveComplete:
            ui->textLog->appendPlainText("The output coefficients are received.");
            break;
        case Service::State::TransmitComplete:
            ui->textLog->appendPlainText("The output coefficients have been transmitted.");
            break;
        }
        break;
    case Services::CoefsIn1:
        switch (st) 
        {
        case Service::State::ReceiveComplete:
            ui->textLog->appendPlainText("The acquisition of the input coefficient on channel 1 is completed.");
            break;
        case Service::State::TransmitComplete:
            ui->textLog->appendPlainText("The input coefficients were transmitted via channel 1.");
            break;
        }
        break;
    case Services::CoefsIn2:
        switch (st) 
        {
        case Service::State::ReceiveComplete:
            ui->textLog->appendPlainText("The acquisition of the input coefficient on channel 2 is completed.");
            break;
        case Service::State::TransmitComplete:
            ui->textLog->appendPlainText("The input coefficients were transmitted via channel 2");
            break;
        }
        break;
    case Services::Parameters:
        in1 = (*measure->Result).V1.RMS;
        in2 = (*measure->Result).V2.RMS;
        phase1 = (*measure->Result).V1.Phase;
        phase2 = (*measure->Result).V2.Phase;
        break;
    }
}
//чтение
void MainWindow::readyRead()
{
    auto data = port->readAll();
    transport->receive(reinterpret_cast<unsigned char*>(data.data()), data.size());
}
//проверка ошибок порта
void MainWindow::portError(QSerialPort::SerialPortError error)
{
    switch (error) {
    case QSerialPort::ResourceError:
        port->close();
        ui->btnConnect->setDisabled(false);
        ui->btnDisconnect->setDisabled(true);
        break;
    default:
        break;
    }
}
//запись
uint8_t MainWindow::Write(std::unique_ptr<addressing_t[]> data, std::size_t size)
{
    return port->write(reinterpret_cast<const char*>(data.get()), size);
}

//обновить порты
void MainWindow::on_btnRefresh_clicked()
{
    ui->lstPort->clear();
    const auto serialPortInfos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& portInfo : serialPortInfos) {
        ui->lstPort->addItem(portInfo.portName(), portInfo.productIdentifier());
    }
}
//подключится
void MainWindow::on_btnConnect_clicked()
{
    try {
        port->setPortName(ui->lstPort->currentText());

        if (port->setBaudRate(7372800) && 
            port->setDataBits(QSerialPort::Data8) && 
            port->setParity(QSerialPort::NoParity) && 
            port->setStopBits(QSerialPort::OneStop) && 
            port->setFlowControl(QSerialPort::HardwareControl)) {
            if (port->open(QIODevice::ReadWrite)) {
                if (port->isOpen()) {
                    dispatcher.resetService();
                    qDebug() << (port->portName() + " >> Open!");
                }
                else {
                    port->close();
                    qDebug() << port->errorString().toLocal8Bit();
                }
            }
        }

        connect(port, &QSerialPort::readyRead, this, &MainWindow::readyRead);

        ui->btnConnect->setDisabled(true);
        ui->btnDisconnect->setDisabled(false);
    }
    catch (...) {
        qDebug("Do not create serial port object");
    }
}
//отключиться
void MainWindow::on_btnDisconnect_clicked()
{
    if (port->isOpen()) {
        qDebug() << (port->portName() + " >> Close!");
        port->close();
    }
    ui->btnConnect->setDisabled(false);
    ui->btnDisconnect->setDisabled(true);
}
//настройки(не работает)
void MainWindow::showSetting(void)
{
    if (out->exec() == QDialog::Accepted) {
        if (port->isOpen()) {
            out->transmit(1, Service::Type::TransmitStreaming);
        }
        else {
            ui->textLog->appendPlainText("Port is not open!");
        }
    }
}
//старт
void MainWindow::on_btnStart_clicked()
{
    if (port->isOpen()) {
        switch (ui->tabWidget->currentIndex()) {
        case 0: ///< Калибровка выхода
            if (ui->rbOutMin->isChecked()) {
                processes.run(ui->lstOutGain->currentData().toUInt() | flagMin);
            }
            else {
                processes.run(ui->lstOutGain->currentData().toUInt());
            }
            break;
        case 1: ///< Калибровка входа
            if (ui->rbIn1->isChecked()) {
                if (ui->rbInMin->isChecked()) {
                    processes.run(ui->lstInGain->currentData().toUInt() | flagInCh1 | flagMin);
                }
                else {
                    processes.run(ui->lstInGain->currentData().toUInt() | flagInCh1);
                }
            }
            else {
                if (ui->rbInMin->isChecked()) {
                    processes.run(ui->lstInGain->currentData().toUInt() | flagMin);
                }
                else {
                    processes.run(ui->lstInGain->currentData().toUInt());
                }
            }
            break;
        }
    }
    else {
        ui->textLog->appendPlainText("Port is not open!");
    }

}
//стоп
void MainWindow::on_btnStop_clicked()
{
    if (port->isOpen()) {
        control->command = controlService::Stop;
        control->transmit(1, Service::Type::TransmitStreaming);
    }
    else {
        ui->textLog->appendPlainText("Port is not open!");
    }
}
//сохранить
void MainWindow::on_btnSave_clicked()
{
    if (port->isOpen()) {
        processes.run(processSave);
    }
    else {
        ui->textLog->appendPlainText("Port is not open!");
    }
}
//загрузить
void MainWindow::on_btnLoad_clicked()
{
    if (port->isOpen()) {
        processes.run(processLoad);
    }
    else {
        ui->textLog->appendPlainText("Port is not open!");
    }
}
//изменения выходного усиления
void MainWindow::on_lstOutGain_currentIndexChanged(int index)
{
    ui->OutMin->setValue(0.f);
    ui->OutMax->setValue(0.f);
}
//коэф. 1го канала
void MainWindow::on_btnCoefIn1_clicked()
{
    if (coefsIn1->exec() == QDialog::Accepted) {
        coefsIn1->updateData();
    }
}
//коэф. 2го канала
void MainWindow::on_btnCoefIn2_clicked()
{
    if (coefsIn2->exec() == QDialog::Accepted) {
        coefsIn2->updateData();
    }
}
//изменение входного усидения
void MainWindow::on_lstInGain_currentIndexChanged(int index)
{
    ui->inMin->setValue(0.f);
    ui->inMax->setValue(0.f);
}
//зафиксировать значения при калибровке входа
void MainWindow::on_btnFixIn_clicked()
{
    if (ui->rbInMin->isChecked()) {
        if (ui->rbIn1->isChecked()) {
            in1_min = in1;
        }
        else {
            in2_min = in2;
        }
        ui->inMin->setValue(ui->inCal->value());
    }
    else {
        if (ui->rbIn1->isChecked()) {
            in1_max = in1;
        }
        else {
            in2_max = in2;
        }
        ui->inMax->setValue(ui->inCal->value());
    }
}
//выбор входного канала
void MainWindow::on_btnCalIn_clicked()
{
    if (ui->rbIn1->isChecked()) {
        calIn(coefsIn1, '1', in1_min, in1_max, phase1);
    }
    else {
        calIn(coefsIn2, '2', in2_min, in2_max, phase2);
    }
}
//Калибровка входа
void MainWindow::calIn(std::shared_ptr<coefInService> coefsIn, char channel, float min, float max, float phase)
{
    switch (ui->lstInGain->currentData().toUInt()) {
    case processIn_x1_8://1:8
        coefsIn->Volt_1_8_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_1_8_b = ui->inMin->value() - min * coefsIn->Volt_1_8_a;
        coefsIn->Phase_1_8 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1/8: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_1_8_a).arg(coefsIn->Volt_1_8_b).arg(coefsIn->Phase_1_8));
        break;
    case processIn_x1_4://1:4
        coefsIn->Volt_1_4_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_1_4_b = ui->inMin->value() - min * coefsIn->Volt_1_4_a;
        coefsIn->Phase_1_4 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1/4: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_1_4_a).arg(coefsIn->Volt_1_4_b).arg(coefsIn->Phase_1_4));
        break;
    case processIn_x1_2://1:2
        coefsIn->Volt_1_2_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_1_2_b = ui->inMin->value() - min * coefsIn->Volt_1_2_a;
        coefsIn->Phase_1_2 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1/2: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_1_2_a).arg(coefsIn->Volt_1_2_b).arg(coefsIn->Phase_1_2));
        break;
    case processIn_x001://1:1
        coefsIn->Volt_001_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_001_b = ui->inMin->value() - min * coefsIn->Volt_001_a;
        coefsIn->Phase_001 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x1: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_001_a).arg(coefsIn->Volt_001_b).arg(coefsIn->Phase_001));
        break;
    case processIn_x002://х2
        coefsIn->Volt_002_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_002_b = ui->inMin->value() - min * coefsIn->Volt_002_a;
        coefsIn->Phase_002 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x2: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_002_a).arg(coefsIn->Volt_002_b).arg(coefsIn->Phase_002));
        break;
    case processIn_x004://х4
        coefsIn->Volt_004_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_004_b = ui->inMin->value() - min * coefsIn->Volt_004_a;
        coefsIn->Phase_004 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x4: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_004_a).arg(coefsIn->Volt_004_b).arg(coefsIn->Phase_004));
        break;
    case processIn_x008://х8
        coefsIn->Volt_008_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_008_b = ui->inMin->value() - min * coefsIn->Volt_008_a;
        coefsIn->Phase_008 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x8: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_008_a).arg(coefsIn->Volt_008_b).arg(coefsIn->Phase_008));
        break;
    case processIn_x016://х16
        coefsIn->Volt_016_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_016_b = ui->inMin->value() - min * coefsIn->Volt_016_a;
        coefsIn->Phase_016 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x16: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_016_a).arg(coefsIn->Volt_016_b).arg(coefsIn->Phase_016));
        break;
    case processIn_x032://х32
        coefsIn->Volt_032_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_032_b = ui->inMin->value() - min * coefsIn->Volt_032_a;
        coefsIn->Phase_032 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x32: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_032_a).arg(coefsIn->Volt_032_b).arg(coefsIn->Phase_032));
        break;
    case processIn_x064://х64
        coefsIn->Volt_064_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_064_b = ui->inMin->value() - min * coefsIn->Volt_064_a;
        coefsIn->Phase_064 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x64: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_064_a).arg(coefsIn->Volt_064_b).arg(coefsIn->Phase_064));
        break;
    case processIn_x128://х128
        coefsIn->Volt_128_a = (ui->inMax->value() - ui->inMin->value()) / (max - min);
        coefsIn->Volt_128_b = ui->inMin->value() - min * coefsIn->Volt_128_a;
        coefsIn->Phase_128 = phase - ui->inCalPhase->value();
        ui->textLog->appendPlainText(QString("Calculated %1 channel coefficients for input gain x128: a=%2; b=%3; phase=%4").arg(channel).arg(coefsIn->Volt_128_a).arg(coefsIn->Volt_128_b).arg(coefsIn->Phase_128));
        break;
    }
}
//выходной коэф.
void MainWindow::on_btnCoefOut_clicked()
{
    if (coefsOut->exec() == QDialog::Accepted) {
        coefsOut->updateData();
    }
}
//зафиксировать значения
void MainWindow::on_btnFixOut_clicked()
{
    if (ui->rbOutMin->isChecked()) {
        ui->OutMin->setValue(ui->outCal->value());
    }
    else {
        ui->OutMax->setValue(ui->outCal->value());
    }
}
//Калибровка выхода
void MainWindow::on_btnCalOut_clicked()
{
    switch (ui->lstOutGain->currentData().toUInt()) {
    case processOut_x1://х1
        coefsOut->Out_x1_a = (ui->OutMax->value() - ui->OutMin->value()) / (3000.f - 2000.f);
        coefsOut->Out_x1_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x1_a;
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain x1: a=%1; b=%2;").arg(coefsOut->Out_x1_a).arg(coefsOut->Out_x1_b));
        break;
    case processOut_x2://х2
        coefsOut->Out_x2_a = (ui->OutMax->value() - ui->OutMin->value()) / (3000.f - 2000.f);
        coefsOut->Out_x2_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x2_a;
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain. x2: a=%1; b=%2;").arg(coefsOut->Out_x2_a).arg(coefsOut->Out_x2_b));
        break;
    case processOut_x4://х4
        coefsOut->Out_x4_a = (ui->OutMax->value() - ui->OutMin->value()) / (3000.f - 2000.f);
        coefsOut->Out_x4_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x4_a;
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain. x4: a=%1; b=%2;").arg(coefsOut->Out_x4_a).arg(coefsOut->Out_x4_b));
        break;
    case processOut_x8://х8
        coefsOut->Out_x8_a = (ui->OutMax->value() - ui->OutMin->value()) / (2650.f - 2000.f);
        coefsOut->Out_x8_b = ui->OutMin->value() - 2000.f * coefsOut->Out_x8_a;
        ui->textLog->insertPlainText(QString("Calculated coefficients for output gain. x8: a=%1; b=%2;").arg(coefsOut->Out_x8_a).arg(coefsOut->Out_x8_b));
        break;
    }
}
//редактирование теста
void MainWindow::on_textLog_textChanged()
{
    QTextCursor cursor = ui->textLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textLog->setTextCursor(cursor);
}
