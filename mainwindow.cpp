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
    stan = false;
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
    connect(ui->start_pushButton, &QPushButton::clicked, this, [=]() {
        if (socket && socket->isOpen()) {
            wyslijKomende("START");
        }
    });

    connect(ui->stop_pushButton, &QPushButton::clicked, this, [=]() {
        if (socket && socket->isOpen()) {
            wyslijKomende("STOP");
        }
    });

    connect(ui->reset_pushButton, &QPushButton::clicked, this, [=]() {
        if (socket && socket->isOpen()) {
            wyslijKomende("RESET");
        }
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
    if (!czyserwer && socket != nullptr) {
        return;
    }

    symulator.uruchomSymulacje();
    double czas = symulator.getAktualnyCzas();
    double wartoscZadana = symulator.getWartoscZadana();
    double wartoscRegulowana = symulator.getWartoscRegulowana();

    double uchyb = wartoscZadana - wartoscRegulowana;
    double sterowanie = symulator.getSterowanie();
    if (czyserwer && socket != nullptr) {
        wyslijWartosc('S', sterowanie);
        wyslijWartosc('t', czas);
        wyslijWartosc('Z', wartoscZadana);
    }

    double sterowanieP = symulator.getSterowanieP();
    double sterowanieI = symulator.getSterowanieI();
    double sterowanieD = symulator.getSterowanieD();

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
        ui->wartosci_wykres->graph(i)->data()->removeBefore(granicaUsuwania);
    for (int i = 0; i < ui->sterowanie_wykres->graphCount(); ++i)
        ui->sterowanie_wykres->graph(i)->data()->removeBefore(granicaUsuwania);
    for (int i = 0; i < ui->uchyb_wykres->graphCount(); ++i)
        ui->uchyb_wykres->graph(i)->data()->removeBefore(granicaUsuwania);

    ui->wartosci_wykres->yAxis->rescale();
    ui->sterowanie_wykres->yAxis->rescale();
    ui->uchyb_wykres->yAxis->rescale();

    ui->wartosci_wykres->replot();
    ui->sterowanie_wykres->replot();
    ui->uchyb_wykres->replot();

    if (!stan && socket != nullptr) {
        oczekiwanyIndeks = aktualnyIndeks;
    }

    stan = false;
}

void MainWindow::aktualizujWykresyARX(double czas, double wartoscZadana, double wartoscRegulowana, double sterowanie)
{
    double zakresCzasu = 10.0;

    if (czas > zakresCzasu) {
        ui->wartosci_wykres->xAxis->setRange(czas - zakresCzasu, czas);
        ui->sterowanie_wykres->xAxis->setRange(czas - zakresCzasu, czas);
    } else {
        ui->wartosci_wykres->xAxis->setRange(0, zakresCzasu);
        ui->sterowanie_wykres->xAxis->setRange(0, zakresCzasu);
    }

    ui->wartosci_wykres->graph(0)->addData(czas, wartoscRegulowana);
    ui->wartosci_wykres->graph(1)->addData(czas, wartoscZadana);
    ui->sterowanie_wykres->graph(0)->addData(czas, sterowanie);

    double granicaUsuwania = czas - zakresCzasu;
    for (int i = 0; i < ui->wartosci_wykres->graphCount(); ++i)
        ui->wartosci_wykres->graph(i)->data()->removeBefore(granicaUsuwania);
    for (int i = 0; i < ui->sterowanie_wykres->graphCount(); ++i)
        ui->sterowanie_wykres->graph(i)->data()->removeBefore(granicaUsuwania);

    ui->wartosci_wykres->yAxis->rescale();
    ui->sterowanie_wykres->yAxis->rescale();

    ui->wartosci_wykres->replot();
    ui->sterowanie_wykres->replot();
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

    regulator.setTi(ui->ti_doubleSpinBox->value());

    regulator.setTd(ui->td_doubleSpinBox->value());

    regulator.setOgraniczenia(ui->uMIN_doubleSpinBox->value(), ui->uMAX_doubleSpinBox->value());
    regulator.setAntiWindup(ui->AntiWindup_checkbox->checkState());
    if(socket!=nullptr)
    {
        wyslijWartosc('P',ui->kp_doubleSpinBox->value());
        wyslijWartosc('I',ui->ti_doubleSpinBox->value());
        wyslijWartosc('D',ui->td_doubleSpinBox->value());
    }

}

void MainWindow::zmienTypSygnalu()
{
    sygnal.setTypSygnalu(ui->typSygnalu_comboBox->currentIndex());
    if(socket!=nullptr)wyslijWartosc('T',ui->typSygnalu_comboBox->currentIndex());
}

void MainWindow::zmienParametrySygnalu()
{
    sygnal.setAmplituda(ui->amplituda_doubleSpinBox->value());   
    sygnal.setOkres(ui->okres_spinBox->value());    
    sygnal.setWypelnienie(ui->wypelnienie_doubleSpinBox->value());
    sygnal.setWartoscStala(ui->stala_spinbox->value());
    sygnal.setChwilaAktywacji(ui->chwilaAktywacji_spinBox->value());
    if(socket!=nullptr)
    {
        wyslijWartosc('A',ui->amplituda_doubleSpinBox->value());
        wyslijWartosc('C',ui->chwilaAktywacji_spinBox->value());
        wyslijWartosc('w',ui->wypelnienie_doubleSpinBox->value());
        wyslijWartosc('O',ui->okres_spinBox->value());
        wyslijWartosc('s',ui->stala_spinbox->value());
    }
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
    //connect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);
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

    static double t = -1.0;
    static double z = 0.0;
    static double u = 0.0;
    static bool t_ok = false, z_ok = false, u_ok = false;

    bufor += socket->readAll();

    while (bufor.size() >= 10) {
        char typ = bufor[0];
        double wartosc;
        memcpy(&wartosc, bufor.constData() + 1, sizeof(double));
        uint8_t indeks = static_cast<uint8_t>(bufor[9]);
        bufor.remove(0, 10);

        switch (typ) {
        case 't': t = wartosc; t_ok = true; break;
        case 'Z': z = wartosc; z_ok = true; break;
        case 'U': u = wartosc; u_ok = true; break;

        case 'S':
            if (!czyserwer) break;

            if (indeks == oczekiwanyIndeks) {
                sprzezenie.setSterowanie(wartosc);
                double y = model.obliczARX(wartosc);
                wyslijWartosc('W', y);

                if (t_ok && z_ok && u_ok) {
                    aktualizujWykresyARX(t, z, y, u);
                }

                oczekiwanyIndeks = (oczekiwanyIndeks + 1) % 256;
                t_ok = z_ok = u_ok = false;
            } else {
                qDebug() << "Błąd indeksu: oczekiwano" << oczekiwanyIndeks << "otrzymano" << indeks;
            }
            break;

        case 'P': ui->kp_doubleSpinBox->setValue(wartosc); break;
        case 'I': ui->ti_doubleSpinBox->setValue(wartosc); break;
        case 'D': ui->td_doubleSpinBox->setValue(wartosc); break;
        case 'T': ui->typSygnalu_comboBox->setCurrentIndex(static_cast<int>(wartosc)); break;
        case 'A': ui->amplituda_doubleSpinBox->setValue(wartosc); break;
        case 's': ui->stala_spinbox->setValue(wartosc); break;
        case 'w': ui->wypelnienie_doubleSpinBox->setValue(wartosc); break;
        case 'O': ui->okres_spinBox->setValue(wartosc); break;
        case 'C': {
            int komenda = static_cast<int>(wartosc);
            switch (komenda) {
            case 0: startSimulation(); break;
            case 1: stopSimulation(); break;
            case 2: resetSimulation(); break;
            default: break;
            }
            break;
        }

        default:
            qDebug() << "Nieznany typ wiadomości:" << typ;
            break;
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
    this->czyserwer = !tryb;

    disconnect(simulationTimer, nullptr, nullptr, nullptr); // usunięcie wcześniejszych przypięć
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);

    if (tryb) {
        startClient();
    } else {
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

    disconnect(simulationTimer, nullptr, nullptr, nullptr);
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);

    stanOffline();
}
void MainWindow::wyslijKomende(const QString &komenda)
{
    if (komenda == "START")
        wyslijWartosc('C', 0.0);
    else if (komenda == "STOP")
        wyslijWartosc('C', 1.0);
    else if (komenda == "RESET")
        wyslijWartosc('C', 2.0);
}

void MainWindow::wyslijWartosc(char kategoria, double wartosc)
{
    if (socket == nullptr) {
        qDebug() << "Nie wysłano: brak socketu lub to serwer.";
        return;
    }

    QByteArray wiadomosc;
    wiadomosc.append(kategoria);
    QByteArray binarnyDouble(reinterpret_cast<const char*>(&wartosc), sizeof(double));
    wiadomosc.append(binarnyDouble);

    uint8_t indeksDoWyslania = aktualnyIndeks;
    if (kategoria != 'C') {
        wiadomosc.append(static_cast<char>(aktualnyIndeks));
        aktualnyIndeks = (aktualnyIndeks + 1) % 256;
    } else {
        wiadomosc.append(static_cast<char>(0)); // komendy mają indeks 0
    }

    qint64 bajty = socket->write(wiadomosc);
    qDebug() << "Wysłano: typ=" << kategoria << ", wartosc=" << wartosc << ", indeks=" << static_cast<int>(indeksDoWyslania);

    socket->flush();
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
