#pragma once
#include <cmath>

class Sygnal
{
    double m_amplituda;
    double m_okres;
    double m_wypelnienie;
    int m_chwilaAktywacji;
    double m_wartoscStala;
    int m_czas;
    int m_typ;
public:
    Sygnal(double amplituda = 0.0, double okres = 0.0, double wypelnienie = 0.0, int chwilaAktywacji = 0.0, double wartoscStala = 0.0, int czas = 0, int typ = 0);
    double getWartoscZadana();
    void setAmplituda(double amplituda);
    void setOkres(double okres);
    void setWypelnienie(double wypelnienie);
    void setChwilaAktywacji(double chwilaAktywacji);
    void setWartoscStala(double wartoscStala);
    void setTypSygnalu(int typ);
    void ustawSkokJednostkowy(double wartoscStala, int chwilaAktywacji);
    void ustawSinusoidalny(double okres, double amplituda);
    void ustawProstokatny(double okres, double amplituda, double wypelnienie);
    void reset();

};
