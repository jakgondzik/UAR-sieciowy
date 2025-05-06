#include "dialogsiec.h"
#include "ui_dialogsiec.h"
#include "QHostAddress"
DialogSiec::DialogSiec(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogSiec)
{
    ui->setupUi(this);
}

DialogSiec::~DialogSiec()
{
    delete ui;
}

void DialogSiec::on_buttonBox_accepted()
{
    QString ipCandidate = QString("%1.%2.%3.%4")
    .arg(ui->spinBoxIP1->value())
        .arg(ui->spinBoxIP2->value())
        .arg(ui->spinBoxIP3->value())
        .arg(ui->spinBoxIP4->value());

    int portCandidate = ui->spinBoxPort->value();

    QHostAddress test;
    if (!test.setAddress(ipCandidate)) {

        return;
    }

    if (portCandidate < 1024 || portCandidate > 65535) {

        return;
    }

    ip = ipCandidate;
    port = portCandidate;
    trybpracy = ui->comboBoxTryb->currentIndex();
    //true server, false klient
    if(trybpracy)
    {

    }
    else
    {

    }
    accept();
}

