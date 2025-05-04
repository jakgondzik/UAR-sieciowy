#include "dialogsiec.h"
#include "ui_dialogsiec.h"

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
    port = ui->spinBoxPort->value();
    ip = QString("%1.%2.%3.%4")
            .arg(ui->spinBoxIP1->value())
            .arg(ui->spinBoxIP2->value())
            .arg(ui->spinBoxIP3->value())
            .arg(ui->spinBoxIP4->value());
    trybpracy = ui->comboBoxTryb->currentIndex();
}

