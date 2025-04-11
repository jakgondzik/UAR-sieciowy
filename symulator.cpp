#include "Symulator.h"
#include <QDebug>


Symulator::Symulator(Sprzezenie& sprzezenie, double czasSymulacji, double krokCzasu)
    : m_sprzezenie(sprzezenie), m_krokCzasu(krokCzasu), m_aktualnyCzas(0.0)
{}

void Symulator::uruchomSymulacje()
{
    m_sprzezenie.wykonajKrok();

    m_aktualnyCzas += m_krokCzasu;
}

void Symulator::reset()
{

    m_aktualnyCzas = 0.0;
    m_sprzezenie.reset();
}

double Symulator::getAktualnyCzas() {return m_aktualnyCzas;}
double Symulator::getWartoscZadana() {return m_sprzezenie.getWartoscZadana();}
double Symulator::getWartoscRegulowana() {return m_sprzezenie.getWartoscRegulowana();}
double Symulator::getSterowanie() {return m_sprzezenie.getSterowanie();}
double Symulator::getSterowanieP() {return m_sprzezenie.getSterowanieP();}
double Symulator::getSterowanieI() {return m_sprzezenie.getSterowanieI();}
double Symulator::getSterowanieD() {return m_sprzezenie.getSterowanieD();}
double Symulator::getUchyb() {return m_sprzezenie.getUchyb();}
