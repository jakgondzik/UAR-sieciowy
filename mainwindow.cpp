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

    ustawWykresy();

}

MainWindow::~MainWindow()
{
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
    connect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);
}

void MainWindow::stopSimulation()
{
    simulationTimer->stop();
    disconnect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);
}

void MainWindow::resetSimulation()
{

    simulationTimer->stop();
    disconnect(simulationTimer, &QTimer::timeout, this, &MainWindow::aktualizujWykresy);

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
    double uchyb = symulator.getWartoscZadana() - symulator.getWartoscRegulowana();
    double sterowanie = symulator.getSterowanie();
    double sterowanieP = symulator.getSterowanieP();
    double sterowanieI = symulator.getSterowanieI();
    double sterowanieD = symulator.getSterowanieD();
    if(serverClientSocket != nullptr && clientSocket != nullptr)
    {
        wyslijDane(czas,wartoscZadana,wartoscRegulowana,sterowanie);
    }
    qDebug() << "Wartość zadana: " << wartoscZadana;
    qDebug() << "Wartość regulowana: " << wartoscRegulowana;
    qDebug() << "Uchyb: " << uchyb;
    qDebug() << "Sterowanie: " << sterowanie;
    qDebug() << "Sterowanie P: " << sterowanieP;
    qDebug() << "Sterowanie I: " << sterowanieI;
    qDebug() << "Sterowanie D: " << sterowanieD;
    qDebug() << "Czas symulacji:" << czas;
    qDebug() << "kP:" << regulator.getKp() << ", Ti:" << regulator.getTi() << ", Td:" << regulator.getTd();

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


    // TU DODAJEMY SIEĆ
    if(clientSocket==nullptr && serverClientSocket == nullptr)
    {

    }
    else if (czyserwer) {
        wyslijDane(czas, wartoscZadana, wartoscRegulowana, sterowanie);
    }
    else
    {
        odbierzDane();
    }
    ui->wartosci_wykres->graph(0)->addData(czas, wartoscRegulowana);
    ui->wartosci_wykres->graph(1)->addData(czas, wartoscZadana);

    ui->sterowanie_wykres->graph(0)->addData(czas, sterowanie);
    ui->sterowanie_wykres->graph(1)->addData(czas, sterowanieP);
    ui->sterowanie_wykres->graph(2)->addData(czas, sterowanieI);
    ui->sterowanie_wykres->graph(3)->addData(czas, sterowanieD);

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
    regulator.setTi(ui->ti_doubleSpinBox->value());
    regulator.setTd(ui->td_doubleSpinBox->value());
    regulator.setOgraniczenia(ui->uMIN_doubleSpinBox->value(), ui->uMAX_doubleSpinBox->value());
    regulator.setAntiWindup(ui->AntiWindup_checkbox->checkState());

}

void MainWindow::zmienTypSygnalu()
{
    sygnal.setTypSygnalu(ui->typSygnalu_comboBox->currentIndex());
}

void MainWindow::zmienParametrySygnalu()
{
    sygnal.setAmplituda(ui->amplituda_doubleSpinBox->value());
    sygnal.setOkres(ui->okres_spinBox->value());
    sygnal.setWypelnienie(ui->wypelnienie_doubleSpinBox->value());
    sygnal.setWartoscStala(ui->stala_spinbox->value());
    sygnal.setChwilaAktywacji(ui->chwilaAktywacji_spinBox->value());
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
}

void MainWindow::onNewConnection() {
    serverClientSocket = server->nextPendingConnection();
    ui->lbStanSieci->setText("Serwer: połączenie z " + serverClientSocket->peerAddress().toString());

    connect(serverClientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(serverClientSocket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);
}

void MainWindow::startClient() {
    clientSocket = new QTcpSocket(this);
    connect(clientSocket, &QTcpSocket::connected, this, &MainWindow::onClientConnected);
    connect(clientSocket, &QTcpSocket::readyRead, this, &MainWindow::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &MainWindow::onDisconnected);

    clientSocket->connectToHost(ip, port);
    ui->lbStanSieci->setText("Klient: łączenie z serwerem...");
}

void MainWindow::onClientConnected() {
    ui->lbStanSieci->setText("Klient: połączono z serwerem.");
    clientSocket->write("Wiadomość od klienta");
}

void MainWindow::onReadyRead() {
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    QByteArray data = socket->readAll();
    ui->lbStanSieci->setText("Odebrano: " + QString(data));
}

void MainWindow::onDisconnected() {
    ui->lbStanSieci->setText("Rozłączono.");
}
void MainWindow::PIDstanKontrolek(bool stan)
{
    ui->typSygnalu_comboBox->setEnabled(stan);
    ui->amplituda_doubleSpinBox->setEnabled(stan);
    ui->stala_spinbox->setEnabled(stan);
    ui->wypelnienie_doubleSpinBox->setEnabled(stan);
    ui->chwilaAktywacji_spinBox->setEnabled(stan);
    ui->okres_spinBox->setEnabled(stan);

}
void MainWindow::ARXstanKontrolek(bool stan)
{
    ui->AntiWindup_checkbox->setEnabled(stan);
    ui->kp_doubleSpinBox->setEnabled(stan);
    ui->ti_doubleSpinBox->setEnabled(stan);
    ui->td_doubleSpinBox->setEnabled(stan);
    ui->wSumie_radioButton->setEnabled(stan);
    ui->przedSuma_radioButton->setEnabled(stan);
    ui->uMAX_doubleSpinBox->setEnabled(stan);
    ui->uMIN_doubleSpinBox->setEnabled(stan);
}
void MainWindow::stanOffline()
{
    PIDstanKontrolek(true);
    ARXstanKontrolek(true);

}


void MainWindow::on_polaczenie_button_clicked()
{
    if(!oknosiec)
    {
        oknosiec = new DialogSiec(this);
        connect(oknosiec, &DialogSiec::PolaczSie, this, &MainWindow::onPolaczSie);
    }

    oknosiec->show();
}

void MainWindow::onPolaczSie(const QString& ip, int port, bool tryb)
{
    this->ip = ip;
    this->port = port;
    this->czyserwer = tryb;

    if(!tryb)
    {
        startClient();
    }
    else
    {
        startServer();
    }
}


void MainWindow::wyslijDane(double czas, double zadana, double regulowana, double sterowanie)
{
    if (!czyserwer || serverClientSocket==nullptr) return;

    QString wiadomosc = QString("%1,%2,%3,%4\n")
                            .arg(czas)
                            .arg(zadana)
                            .arg(regulowana)
                            .arg(sterowanie);
    serverClientSocket->write(wiadomosc.toUtf8());
}

void MainWindow::odbierzDane()
{

    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket || !socket->bytesAvailable()) return;

    QByteArray dane = socket->readAll();
    QList<QByteArray> linie = dane.split('\n');

    for (const QByteArray &linia : linie) {
        if (linia.trimmed().isEmpty()) continue;

        if (czyserwer && socket == serverClientSocket) {
            // Odbieranie wartości regulowanej od klienta
            bool ok;
            double wartoscRegulowanaOdebrana = linia.toDouble(&ok);
            if (ok) {
                qDebug() << "[SERWER ODB] Wartość regulowana od klienta:" << wartoscRegulowanaOdebrana;
                //symulator.setWartoscRegulowanaZSieci(wartoscRegulowanaOdebrana);
            } else {
                qDebug() << "[SERWER ODB] Błąd konwersji wartości regulowanej od klienta.";
            }
        } else if (!czyserwer && socket == clientSocket) {
            // Odbieranie danych z serwera
            QList<QByteArray> pola = linia.split(',');

            if (pola.size() == 4) {
                bool ok1, ok2, ok3, ok4;
                double czasOdebrany = pola[0].toDouble(&ok1);
                double zadanaOdebrana = pola[1].toDouble(&ok2);
                double regulowanaOdebranaZSerwera = pola[2].toDouble(&ok3);
                double sterowanieOdebrane = pola[3].toDouble(&ok4);

                if (ok1 && ok2 && ok3 && ok4) {
                    qDebug() << "[KLIENT ODB] czas:" << czasOdebrany << "zadana:" << zadanaOdebrana
                             << "regulowana (z serwera):" << regulowanaOdebranaZSerwera << "sterowanie:" << sterowanieOdebrane;

                    // Aktualizacja lokalnych wykresów klienta
                    if(czasPoprzedni != czasOdebrany)
                    ui->wartosci_wykres->graph(0)->addData(czasOdebrany, regulowanaOdebranaZSerwera);
                    ui->wartosci_wykres->graph(1)->addData(czasOdebrany, zadanaOdebrana);
                    ui->wartosci_wykres->xAxis->rescale();
                    ui->wartosci_wykres->yAxis->rescale();
                    ui->wartosci_wykres->replot();

                    // Symulacja obiektu ARX
                    double wartoscRegulowanaObiektu = model.obliczARX(sterowanieOdebrane);
                    czasPoprzedni = czasOdebrany;
                    // Wysyłanie wartości regulowanej obiektu do serwera
                    //wyslijWartoscRegulowana(wartoscRegulowanaObiektu);
                }
            }
        }
    }
}

void MainWindow::wyslijWartoscRegulowana(double wartoscRegulowana)
{
    if (czyserwer || clientSocket == nullptr) return;

    QString wiadomosc = QString("%1\n").arg(wartoscRegulowana);
    clientSocket->write(wiadomosc.toUtf8());
}
