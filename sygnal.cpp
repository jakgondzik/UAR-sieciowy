#include "sygnal.h"

Sygnal::Sygnal(double amplituda, double okres, double wypelnienie, int chwilaAktywacji, double wartoscStala, int czas, int typ)
    :m_amplituda(amplituda), m_okres(okres), m_wypelnienie(wypelnienie), m_chwilaAktywacji(chwilaAktywacji), m_wartoscStala(wartoscStala), m_czas(czas) ,m_typ(typ)
{}

void Sygnal::ustawSkokJednostkowy(double wartoscStala, int chwilaAktywacji)
{
    m_typ = 0;
    m_wartoscStala = wartoscStala;
    m_chwilaAktywacji = chwilaAktywacji;
}

void Sygnal::ustawSinusoidalny(double okres, double amplituda)
{
    m_typ = 1;
    m_okres = okres;
    m_amplituda = amplituda;
}

void Sygnal::ustawProstokatny(double okres, double amplituda, double wypelnienie)
{
    m_typ = 2;
    m_okres = okres;
    m_amplituda = amplituda;
    m_wypelnienie = wypelnienie;
}

double Sygnal::getWartoscZadana()
{
    double wartosc = 0.0;

    switch (m_typ)
    {
    case 0:
        if (m_czas >= m_chwilaAktywacji) wartosc = m_wartoscStala;
        break;
    case 1:
        wartosc = m_amplituda * sin(2 * 3.14 * (m_czas % static_cast<int>(m_okres)) / m_okres);
        break;
    case 2:
        if ((m_czas % static_cast<int>(m_okres)) < (m_wypelnienie * m_okres))
        {
            wartosc = m_amplituda;
        }
        break;
    }
    m_czas++;
    return wartosc;

}

void Sygnal::setAmplituda(double amplituda) { m_amplituda = amplituda; }
void Sygnal::setOkres(double okres) { m_okres = okres; }
void Sygnal::setWypelnienie(double wypelnienie) { m_wypelnienie = wypelnienie; }
void Sygnal::setChwilaAktywacji(double chwilaAktywacji) { m_chwilaAktywacji = chwilaAktywacji; }
void Sygnal::setWartoscStala(double wartoscStala) { m_wartoscStala = wartoscStala; }
void Sygnal::setTypSygnalu(int typ) { m_typ = typ; }

void Sygnal::reset()
{
    m_amplituda = 0.0;
    m_okres = 0.0;
    m_wypelnienie = 0.0;
    m_chwilaAktywacji = 0.0;
    m_wartoscStala = 0.0;
    m_czas = 0;
    m_typ = 0;
}

