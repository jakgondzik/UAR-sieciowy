#ifndef DIALOGSIEC_H
#define DIALOGSIEC_H

#include <QDialog>

namespace Ui {
class DialogSiec;
}

class DialogSiec : public QDialog
{
    Q_OBJECT

public:
    explicit DialogSiec(QWidget *parent = nullptr);
    ~DialogSiec();

private slots:
    void on_buttonBox_accepted();

signals:
    void PolaczSie(const QString& ip, int port, bool tryb);

private:
    Ui::DialogSiec *ui;
    QString ip;
    int port;
    bool trybpracy;
};



#endif // DIALOGSIEC_H
