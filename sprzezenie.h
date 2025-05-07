#pragma once
#include "sygnal.h"
#include "regulatorPID.h"
#include "modelARX.h"
#include <cmath>

class Sprzezenie {

    Sygnal& m_sygnal;
    RegulatorPID& m_regulator;
    ModelARX& m_model;
    double m_krok;
    double m_aktualnyCzas;
    double m_sterowanie;
    double m_wartoscRegulowana;
    double m_uchyb;
    double m_poprzedniaWartoscRegulowana;
public:
    Sprzezenie(Sygnal& sygnal, RegulatorPID& regulator, ModelARX& model, double krok = 0.1);
    void setRegulator(double Kp, double Ti, double Td);
    void setModel(std::vector<double>& a, std::vector<double>& b, int opoznienie, double odch);
    void setSygnal(double amplituda, double okres, double wypelnienie, double chwilaAktywacji, double wartoscStala, int typ);
    double wykonajKrok();
    double getSterowanie();
    double getSterowanieP();
    double getSterowanieI();
    double getSterowanieD();
    double getWartoscRegulowana();
    void setWartoscRegulowana(double val);
    double getUchyb();
    double getWartoscZadana();
    double getAktualnyCzas();
    void reset();
    void setSterowanie(double val);
};

