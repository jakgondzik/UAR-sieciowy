#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sygnal(5.0, 100.0, 0.5, 2.0, 5.0, 0.0, 1)
    , regulator(1.0, 2.5, 0.2, -5.0, 5.0, false)
    , model({-0.4, 0.0, 0.0}, {0.6, 0.0, 0.0}, 1, 0.01)
    , sprzezenie(sygnal, regulator, model)
    , symulator(sprzezenie, 10.0, 0.1)
    , aktualnyCzas(0.0)
{
    ui->setupUi(this);
    grupaSieciowa = new QButtonGroup(this);

    connect(ui->kp_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametryPID);
    connect(ui->ti_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametryPID);
    connect(ui->td_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametryPID);
    connect(ui->uMIN_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametryPID);
    connect(ui->uMAX_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametryPID);
    connect(ui->AntiWindup_checkbox, &QCheckBox::stateChanged, this, &MainWindow::zmienParametryPID);

    connect(ui->amplituda_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametrySygnalu);
    connect(ui->okres_spinBox, &QSpinBox::editingFinished, this, &MainWindow::zmienParametrySygnalu);
    connect(ui->wypelnienie_doubleSpinBox, &QDoubleSpinBox::editingFinished, this, &MainWindow::zmienParametrySygnalu);
    connect(ui->stala_spinbox, &QSpinBox::editingFinished, this, &MainWindow::zmienParametrySygnalu);
    connect(ui->chwilaAktywacji_spinBox, &QSpinBox::editingFinished, this, &MainWindow::zmienParametrySygnalu);
    connect(ui->typSygnalu_comboBox, &QComboBox::currentIndexChanged, this, &MainWindow::zmienTypSygnalu);

    connect(ui->start_pushButton, &QPushButton::clicked, this, &MainWindow::startSimulation);
    connect(ui->stop_pushButton, &QPushButton::clicked, this, &MainWindow::stopSimulation);
    connect(ui->reset_pushButton, &QPushButton::clicked, this, &MainWindow::resetSimulation);
    //connect(ui->aktualizuj_pushButton, &QPushButton::clicked, this, &MainWindow::aktualizujParametry);

    simulationTimer = new QTimer(this);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);

    ustawWykresy();

    //
    //  SIEĆ
    //
    // Wysyłanie komend po stronie serwera (np. z PID)
    connect(ui->start_pushButton, &QPushButton::clicked, this, [=]() {
        if (socket && socket->isOpen()) {
            socket->write("C:START\n");
        }
     //   startSimulation(); // lokalnie też startujemy
    });

    connect(ui->stop_pushButton, &QPushButton::clicked, this, [=]() {
        if (socket && socket->isOpen()) {
            socket->write("C:STOP\n");
        }
       // stopSimulation();
    });

    connect(ui->reset_pushButton, &QPushButton::clicked, this, [=]() {
        if (socket && socket->isOpen()) {
            socket->write("C:RESET\n");
        }
       // resetSimulation();
    });
}

MainWindow::~MainWindow()
{
    onRozlacz();
    delete ui;
}

void MainWindow::ustawWykresy()
{
    ui->wartosci_wykres->legend->setVisible(true);
    ui->wartosci_wykres->legend->setFont(QFont("Helvetica", 9));

    ui->wartosci_wykres->addGraph();
    ui->wartosci_wykres->graph(0)->setName("Wartość regulowana");
    ui->wartosci_wykres->graph(0)->setPen(QPen(Qt::magenta));

    ui->wartosci_wykres->addGraph();
    ui->wartosci_wykres->graph(1)->setName("Wartość zadana");
    ui->wartosci_wykres->graph(1)->setPen(QPen(Qt::black));


    ui->sterowanie_wykres->legend->setVisible(true);
    ui->sterowanie_wykres->legend->setFont(QFont("Helvetica", 9));

    ui->sterowanie_wykres->addGraph();
    ui->sterowanie_wykres->graph(0)->setName("Sterowanie U");
    ui->sterowanie_wykres->graph(0)->setPen(QPen(Qt::black));

    ui->sterowanie_wykres->addGraph();
    ui->sterowanie_wykres->graph(1)->setName("Sterowanie Up");
    ui->sterowanie_wykres->graph(1)->setPen(QPen(Qt::red));

    ui->sterowanie_wykres->addGraph();
    ui->sterowanie_wykres->graph(2)->setName("Sterowanie Ui");
    ui->sterowanie_wykres->graph(2)->setPen(QPen(Qt::green));

    ui->sterowanie_wykres->addGraph();
    ui->sterowanie_wykres->graph(3)->setName("Sterowanie Ud");
    ui->sterowanie_wykres->graph(3)->setPen(QPen(Qt::blue));


    ui->uchyb_wykres->legend->setVisible(true);
    ui->uchyb_wykres->legend->setFont(QFont("Helvetica", 9));

    ui->uchyb_wykres->addGraph();
    ui->uchyb_wykres->graph(0)->setName("Uchyb");
    ui->uchyb_wykres->graph(0)->setPen(QPen(Qt::darkYellow));
}


void MainWindow::startSimulation()
{
    simulationTimer->start(ui->interwal_spinBox->value());
}

void MainWindow::stopSimulation()
{
    simulationTimer->stop();
}

void MainWindow::resetSimulation()
{

    simulationTimer->stop();

    symulator.reset();
    aktualnyCzas = 0.0;

    for (int i = 0; i < ui->wartosci_wykres->graphCount(); ++i) {
        ui->wartosci_wykres->graph(i)->data()->clear();
    }
    for (int i = 0; i < ui->sterowanie_wykres->graphCount(); ++i) {
        ui->sterowanie_wykres->graph(i)->data()->clear();
    }
    for (int i = 0; i < ui->uchyb_wykres->graphCount(); ++i) {
        ui->uchyb_wykres->graph(i)->data()->clear();
    }

    ui->wartosci_wykres->xAxis->setRange(0, 0);
    ui->sterowanie_wykres->xAxis->setRange(0, 0);
    ui->uchyb_wykres->xAxis->setRange(0, 0);

    ui->wartosci_wykres->yAxis->rescale();
    ui->sterowanie_wykres->yAxis->rescale();
    ui->uchyb_wykres->yAxis->rescale();

    ui->wartosci_wykres->replot();
    ui->sterowanie_wykres->replot();
    ui->uchyb_wykres->replot();

}

void MainWindow::aktualizujWykresy()
{
    symulator.uruchomSymulacje();
    double czas = symulator.getAktualnyCzas();
    double wartoscZadana = symulator.getWartoscZadana();
    double wartoscRegulowana = symulator.getWartoscRegulowana();
    if(socket != nullptr)
    {
        wyslijWartosc('W',wartoscRegulowana);
    }

    double uchyb = symulator.getWartoscZadana() - symulator.getWartoscRegulowana();
    double sterowanie = symulator.getSterowanie();
    if(socket!= nullptr)
    {
        wyslijWartosc('S',sterowanie);
    }

    double sterowanieP = symulator.getSterowanieP();
    double sterowanieI = symulator.getSterowanieI();
    double sterowanieD = symulator.getSterowanieD();
   // qDebug() << "Wartość zadana: " << wartoscZadana;
   // qDebug() << "Wartość regulowana: " << wartoscRegulowana;
   // qDebug() << "Uchyb: " << uchyb;
    //qDebug() << "Sterowanie: " << sterowanie;
   // qDebug() << "Sterowanie P: " << sterowanieP;
   // qDebug() << "Sterowanie I: " << sterowanieI;
  //  qDebug() << "Sterowanie D: " << sterowanieD;
   // qDebug() << "Czas symulacji:" << czas;
   // qDebug() << "kP:" << regulator.getKp() << ", Ti:" << regulator.getTi() << ", Td:" << regulator.getTd();

    double zakresCzasu = 10.0;

    if (czas > zakresCzasu) {
        ui->wartosci_wykres->xAxis->setRange(czas - zakresCzasu, czas);
        ui->sterowanie_wykres->xAxis->setRange(czas - zakresCzasu, czas);
        ui->uchyb_wykres->xAxis->setRange(czas - zakresCzasu, czas);
    } else {
        ui->wartosci_wykres->xAxis->setRange(0, zakresCzasu);
        ui->sterowanie_wykres->xAxis->setRange(0, zakresCzasu);
        ui->uchyb_wykres->xAxis->setRange(0, zakresCzasu);
    }
    ui->sterowanie_wykres->graph(1)->addData(czas, sterowanieP);
    ui->sterowanie_wykres->graph(2)->addData(czas, sterowanieI);
    ui->sterowanie_wykres->graph(3)->addData(czas, sterowanieD);

        ui->sterowanie_wykres->graph(0)->addData(czas, sterowanie);

        ui->wartosci_wykres->graph(0)->addData(czas, wartoscRegulowana);
        ui->wartosci_wykres->graph(1)->addData(czas, wartoscZadana);
        ui->uchyb_wykres->graph(0)->addData(czas, uchyb);





    double granicaUsuwania = czas - zakresCzasu;

    for (int i = 0; i < ui->wartosci_wykres->graphCount(); ++i)
    {
        ui->wartosci_wykres->graph(i)->data()->removeBefore(granicaUsuwania);
    }
    for (int i = 0; i < ui->sterowanie_wykres->graphCount(); ++i)
    {
        ui->sterowanie_wykres->graph(i)->data()->removeBefore(granicaUsuwania);
    }
    for (int i = 0; i < ui->uchyb_wykres->graphCount(); ++i)
    {
        ui->uchyb_wykres->graph(i)->data()->removeBefore(granicaUsuwania);
    }


    ui->wartosci_wykres->yAxis->rescale();
    ui->sterowanie_wykres->yAxis->rescale();
    ui->uchyb_wykres->yAxis->rescale();

    ui->wartosci_wykres->replot();
    ui->sterowanie_wykres->replot();
    ui->uchyb_wykres->replot();


}

void MainWindow::zmienParametryARX(std::vector<double> newA, std::vector<double> newB, int newK, double newOdchStan)
{
    A = newA;
    B = newB;
    opoznienie = newK;
    odchStan = newOdchStan;

    model.setParametryARX(A, B, opoznienie, odchStan);
}

void MainWindow::zmienParametryPID()
{
    regulator.setKp(ui->kp_doubleSpinBox->value());
    wyslijWartosc('P',ui->kp_doubleSpinBox->value());
    regulator.setTi(ui->ti_doubleSpinBox->value());
    wyslijWartosc('I',ui->ti_doubleSpinBox->value());
    regulator.setTd(ui->td_doubleSpinBox->value());
    wyslijWartosc('D',ui->td_doubleSpinBox->value());
    regulator.setOgraniczenia(ui->uMIN_doubleSpinBox->value(), ui->uMAX_doubleSpinBox->value());
    regulator.setAntiWindup(ui->AntiWindup_checkbox->checkState());

}

void MainWindow::zmienTypSygnalu()
{
    sygnal.setTypSygnalu(ui->typSygnalu_comboBox->currentIndex());
    wyslijWartosc('T',ui->typSygnalu_comboBox->currentIndex());
}

void MainWindow::zmienParametrySygnalu()
{
    sygnal.setAmplituda(ui->amplituda_doubleSpinBox->value());
    wyslijWartosc('A',ui->amplituda_doubleSpinBox->value());
    sygnal.setOkres(ui->okres_spinBox->value());
    wyslijWartosc('O',ui->okres_spinBox->value());
    sygnal.setWypelnienie(ui->wypelnienie_doubleSpinBox->value());
    wyslijWartosc('w',ui->wypelnienie_doubleSpinBox->value());
    sygnal.setWartoscStala(ui->stala_spinbox->value());
    wyslijWartosc('s',ui->stala_spinbox->value());
    sygnal.setChwilaAktywacji(ui->chwilaAktywacji_spinBox->value());
    wyslijWartosc('C',ui->chwilaAktywacji_spinBox->value());
}

void MainWindow::aktualizujParametry()
{
    zmienParametryPID();
 //   zmienParametryARX(A, B, opoznienie, odchStan);
    zmienTypSygnalu();
    simulationTimer->start(ui->interwal_spinBox->value());
}

void MainWindow::on_uMAX_doubleSpinBox_valueChanged(double wartosc)
{
    regulator.setOgraniczenia(ui->uMIN_doubleSpinBox->value(), wartosc);
}


void MainWindow::on_uMIN_doubleSpinBox_valueChanged(double wartosc)
{
    regulator.setOgraniczenia(wartosc, ui->uMAX_doubleSpinBox->value());
}


void MainWindow::on_AntiWindup_checkbox_stateChanged(int stan)
{
    regulator.setAntiWindup(stan == Qt::Checked);
}


void MainWindow::on_parametryARX_pushButton_clicked()
{
    if(!m_okno)
    {
        m_okno = new Dialog(this);
        connect(m_okno, &Dialog::aktualizacjaParametrow, this, &MainWindow::zmienParametryARX);
    }

    m_okno->setParameters(A, B, opoznienie, odchStan);
    m_okno->show();
}


void MainWindow::on_przedSuma_radioButton_clicked()
{
    regulator.setTrybCalkowania(TrybCalkowania::STALA_PRZED_SUMA);
}




void MainWindow::on_wSumie_radioButton_clicked()
{
    regulator.setTrybCalkowania(TrybCalkowania::STALA_W_SUMIE);
}


void MainWindow::startServer() {
    server = new QTcpServer(this);
    connect(server, &QTcpServer::newConnection, this, &MainWindow::onNewConnection);

    if (server->listen(QHostAddress::AnyIPv4, port)) {
        ui->lbStanSieci->setText("Serwer: nasłuchiwanie na porcie " + QString::number(port) +".");
    } else {
        ui->lbStanSieci->setText("Błąd serwera: " + server->errorString());
    }
    ARXstanKontrolek(true);
    PIDstanKontrolek(false);
}

void MainWindow::onNewConnection() {
    socket = server->nextPendingConnection();
    ui->lbStanSieci->setText("Serwer: połączenie z " + socket->peerAddress().toString());

    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
}

void MainWindow::startClient() {
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &MainWindow::onClientConnected);
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);

    socket->connectToHost(ip, port);
    ui->lbStanSieci->setText("Klient: łączenie z serwerem...");

    ARXstanKontrolek(false);
    PIDstanKontrolek(true);
}

void MainWindow::onClientConnected() {
    ui->lbStanSieci->setText("Klient: połączono z serwerem.");


}

/*void MainWindow::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray data = socket->readAll();
    qDebug() << "ODEBRANO:" << data;
    QList<QByteArray> pola = data.split(':');

    if(QString(pola[0]) == 'C')
    {
        if (pola[1] == "START") {
            startSimulation();
            return;
        } else if (pola[1] == "STOP") {
            stopSimulation();
            return;
        } else if (pola[1] == "RESET") {
            resetSimulation();
            return;
        }
    }

    if(QString(pola[0]) == 'W')
    {

        sprzezenie.setWartoscRegulowana(pola[1].toDouble());
    }
    if(QString(pola[0]) == 'S')
    {
        sprzezenie.setSterowanie(pola[1].toDouble());
        double y = model.obliczARX(pola[1].toDouble());
        wyslijWartosc('W',y);
    }
   // ui->lbStanSieci->setText("Odebrano: " + pola[1]);
}*/
void MainWindow::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    static QByteArray bufor;

    bufor += socket->readAll();
    qDebug() << "BUFOR: " << bufor;
    // Obsłuż WSZYSTKIE kompletne linie
    int index;
    while ((index = bufor.indexOf('\n')) != -1) {
        QByteArray linia = bufor.left(index).trimmed();
        bufor.remove(0, index + 1);

        qDebug() << "ODEBRANO:" << linia;

        QList<QByteArray> pola = linia.split(':');
        if (pola.size() < 2) continue;

        QByteArray typ = pola[0];
        QByteArray wartosc = pola[1];

        if (typ == "C") {
            if (wartosc == "START") {
                startSimulation();
            } else if (wartosc == "STOP") {
                stopSimulation();
            } else if (wartosc == "RESET") {
                resetSimulation();
            }
        }
        else if (typ == "W") {
            sprzezenie.setWartoscRegulowana(wartosc.toDouble());
        }
        else if (typ == "S") {
            sprzezenie.setSterowanie(wartosc.toDouble());
            double y = model.obliczARX(wartosc.toDouble());
            wyslijWartosc('W', y);
        }
        else if(typ =="P")
        {
            ui->kp_doubleSpinBox->setValue(wartosc.toDouble());
        }
        else if(typ =="I")
        {
            ui->ti_doubleSpinBox->setValue(wartosc.toDouble());
        }
        else if(typ =="D")
        {
            ui->td_doubleSpinBox->setValue(wartosc.toDouble());
        }
        else if(typ =="i")
        {
            ui->wSumie_radioButton->setChecked(true);
        }
        else if(typ =="o")
        {
            ui->przedSuma_radioButton->setChecked(true);
        }
        //ANTY WINDUP NIE DZIALA!!!
        else if(typ =="T")
        {
            if(wartosc.toDouble() == 0)
            {
                ui->typSygnalu_comboBox->setCurrentIndex(0);
            }
            else if(wartosc.toDouble() == 1.0)
            {
                ui->typSygnalu_comboBox->setCurrentIndex(1);
            }
            else if(wartosc.toDouble() == 2.0)
            {
                ui->typSygnalu_comboBox->setCurrentIndex(2);
            }
        }
        else if(typ =="A")
        {
            ui->amplituda_doubleSpinBox->setValue(wartosc.toDouble());
        }
        else if(typ =="s")
        {
            ui->stala_spinbox->setValue(wartosc.toDouble());
        }
        else if(typ =="w")
        {
            ui->wypelnienie_doubleSpinBox->setValue(wartosc.toDouble());
        }
        else if(typ == "C")
        {
            ui->chwilaAktywacji_spinBox->setValue(wartosc.toInt());
            qDebug() << "zapisuje: " << wartosc.toInt();
        }
        else if(typ =="O")
        {
            ui->okres_spinBox->setValue(wartosc.toDouble());
        }

    }
}

void MainWindow::onDisconnected() {

        stanOffline();
}
void MainWindow::PIDstanKontrolek(bool stan)
{

    ui->parametryARX_pushButton->setEnabled(stan);

}
void MainWindow::ARXstanKontrolek(bool stan)
{
    ui->typSygnalu_comboBox->setEnabled(stan);
    ui->amplituda_doubleSpinBox->setEnabled(stan);
    ui->stala_spinbox->setEnabled(stan);
    ui->wypelnienie_doubleSpinBox->setEnabled(stan);
    ui->chwilaAktywacji_spinBox->setEnabled(stan);
    ui->okres_spinBox->setEnabled(stan);
    ui->AntiWindup_checkbox->setEnabled(stan);
    ui->kp_doubleSpinBox->setEnabled(stan);
    ui->ti_doubleSpinBox->setEnabled(stan);
    ui->td_doubleSpinBox->setEnabled(stan);
    ui->wSumie_radioButton->setEnabled(stan);
    ui->przedSuma_radioButton->setEnabled(stan);
    ui->uMAX_doubleSpinBox->setEnabled(stan);
    ui->uMIN_doubleSpinBox->setEnabled(stan);
    ui->interwal_spinBox->setEnabled(stan);
    ui->start_pushButton->setEnabled(stan);
    ui->stop_pushButton->setEnabled(stan);
    ui->reset_pushButton->setEnabled(stan);
}
void MainWindow::stanOffline()
{
    PIDstanKontrolek(true);
    ARXstanKontrolek(true);
    ui->lbStanSieci->setText("Rozłączono.");

}


void MainWindow::on_polaczenie_button_clicked()
{
    if(!oknosiec)
    {
        oknosiec = new DialogSiec(this);
        connect(oknosiec, &DialogSiec::PolaczSie, this, &MainWindow::onPolaczSie);
        connect(oknosiec, &DialogSiec::Rozlacz, this, &MainWindow::onRozlacz);
    }

    oknosiec->show();
}

void MainWindow::onPolaczSie(const QString& ip, int port, bool tryb)
{
    this->ip = ip;
    this->port = port;
    this->czyserwer = tryb;

    if(tryb)
    {
        startClient();
    }
    else
    {
        startServer();
    }
}

void MainWindow::onRozlacz()
{
    if (socket) {
        socket->abort();
        socket->deleteLater();
        socket = nullptr;
    }

    if (server) {
        server->close();
        server->deleteLater();
        server = nullptr;
    }
    stanOffline();

}


void MainWindow::wyslijWartosc(const char kategoria, double wartosc)
{

        if (czyserwer || socket == nullptr) return;

        QByteArray wiadomosc;
        wiadomosc.append(kategoria);
        wiadomosc.append(':');
        wiadomosc.append(QByteArray::number(wartosc, 'f'));
        wiadomosc.append('\n');
        socket->write(wiadomosc);
}
/*
  void MainWindow::wyslijWartosc(char kategoria, double wartosc)
{
    if (czyserwer || socket == nullptr) return;

    QString wiadomosc = kategoria + ":" + QByteArray::number(wartosc);;
    socket->write(wiadomosc.toUtf8());
}
 */
double MainWindow::odbierzRegulowana()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket || !socket->bytesAvailable()) return 0;

    QByteArray dane = socket->readAll();
    return dane.toDouble();
}
