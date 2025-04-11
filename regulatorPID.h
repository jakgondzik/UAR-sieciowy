#pragma once

enum class TrybCalkowania { STALA_PRZED_SUMA, STALA_W_SUMIE };

class RegulatorPID
{
    double m_kp;
    double m_Ti;
    double m_Td;
    double m_sumaUchybow;
    double m_poprzedniUchyb;
    double algorytmP(double, double);
    double algorytmI(double, double);
    double algorytmD(double, double);
    double m_uP;
    double m_uI;
    double m_uD;
    double m_U;
    double m_uMin = -50;
    double m_uMax = 50;
    bool m_antiWindupWlaczony;
    TrybCalkowania m_trybCalkowania = TrybCalkowania::STALA_PRZED_SUMA;
public:
    RegulatorPID(double kp, double ti, double td, double uMin, double uMax, bool antiWindupEnabled = true);
    void setKp(double);
    void setTi(double);
    void setTd(double);
    void setNastawy(double kp, double ti, double  td);
    void resetPamieci();
    void setOgraniczenia(double uMin, double uMax);
    void setAntiWindup(double wlaczony);
    void setTrybCalkowania(TrybCalkowania tryb);
    double Sterowanie(double zadana, double regulowana);
    double getUP();
    double getUI();
    double getUD();
    double getKp();
    double getTi();
    double getTd();

};
