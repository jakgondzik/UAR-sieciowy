#include "Sprzezenie.h"

class Symulator
{
    Sprzezenie& m_sprzezenie;
    double m_krokCzasu;
    double m_aktualnyCzas;
    bool m_aktywnaSymulacja;

public:
    Symulator(Sprzezenie& sprzezenie, double czasSymulacji, double krokCzasu);
    void uruchomSymulacje();
    void stop();
    void reset();
    double getAktualnyCzas();
    double getWartoscZadana();
    double getWartoscRegulowana();
    double getSterowanie();
    double getSterowanieP();
    double getSterowanieI();
    double getSterowanieD();
    double getUchyb();
    Sprzezenie& getSprzezenie();
};
