#include "Sprzezenie.h"


Sprzezenie::Sprzezenie(Sygnal& sygnal, RegulatorPID& regulator, ModelARX& model, double krok)
    : m_sygnal(sygnal),
    m_regulator(regulator),
    m_model(model),
    m_krok(krok),
    m_aktualnyCzas(0.0),
    m_sterowanie(0.0),
    m_wartoscRegulowana(0.0),
    m_uchyb(0.0),
    m_poprzedniaWartoscRegulowana(0.0)
{}

double Sprzezenie::wykonajKrok()
{
    double zadana = m_sygnal.getWartoscZadana();

    m_sterowanie = m_regulator.Sterowanie(zadana, m_wartoscRegulowana);

    m_wartoscRegulowana = m_model.obliczARX(m_sterowanie);

    m_poprzedniaWartoscRegulowana = m_wartoscRegulowana;

    return m_wartoscRegulowana;

}


void Sprzezenie::reset()
{
    m_aktualnyCzas = 0.0;
    m_uchyb = 0.0;
    m_sterowanie = 0.0;
    m_wartoscRegulowana = 0.0;
    m_poprzedniaWartoscRegulowana = 0.0;

    m_model.reset();
    m_regulator.resetPamieci();
}

void Sprzezenie::setRegulator(double Kp, double Ti, double Td)
{
    m_regulator.setNastawy(Kp, Ti, Td);
}

void Sprzezenie::setModel(std::vector<double>& a, std::vector<double>& b, int opoznienie, double odch)
{
    m_model.setParametryARX(a, b, opoznienie, odch);
}

void Sprzezenie::setSygnal(double amplituda, double okres, double wypelnienie, double chwilaAktywacji, double wartoscStala, int typ)
{
    m_sygnal.setAmplituda(amplituda);
    m_sygnal.setOkres(okres);
    m_sygnal.setWypelnienie(wypelnienie);
    m_sygnal.setChwilaAktywacji(chwilaAktywacji);
    m_sygnal.setWartoscStala(wartoscStala);
    m_sygnal.setTypSygnalu(typ);
}

double Sprzezenie::getWartoscRegulowana() { return m_wartoscRegulowana; }
double Sprzezenie::getUchyb() { return m_uchyb; }
double Sprzezenie::getSterowanie() { return m_sterowanie; }
double Sprzezenie::getSterowanieP() { return m_regulator.getUP(); }
double Sprzezenie::getSterowanieI() { return m_regulator.getUI(); }
double Sprzezenie::getSterowanieD() { return m_regulator.getUD(); }
double Sprzezenie::getWartoscZadana() { return m_sygnal.getWartoscZadana(); }
double Sprzezenie::getAktualnyCzas() { return m_aktualnyCzas; }
