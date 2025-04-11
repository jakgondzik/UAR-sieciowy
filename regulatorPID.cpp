#include "regulatorPID.h"
#include <algorithm>
#include <stdexcept>


RegulatorPID::RegulatorPID(double kp, double ti, double td, double uMin, double uMax, bool antiWindup)
    : m_kp(kp), m_Ti(ti), m_Td(td), m_sumaUchybow(0.0), m_poprzedniUchyb(0.0),
    m_uP(0.0), m_uI(0.0), m_uD(0.0), m_U(0.0),
    m_uMin(uMin), m_uMax(uMax), m_antiWindupWlaczony(antiWindup) {}

void RegulatorPID::setKp(double kp) { m_kp = kp;  }
void RegulatorPID::setTi(double ti) { m_Ti = ti; }
void RegulatorPID::setTd(double td) { m_Td = td; }
void RegulatorPID::setNastawy(double kp, double ti, double td)
{
    setKp(kp);
    setTi(ti);
    setTd(td);
}
void RegulatorPID::resetPamieci()
{
    m_sumaUchybow = 0.0;
    m_poprzedniUchyb = 0.0;
}
void RegulatorPID::setTrybCalkowania(TrybCalkowania tryb)
{
    m_trybCalkowania = tryb;
}

void RegulatorPID::setOgraniczenia(double uMin, double uMax)
{
    m_uMin = uMin;
    m_uMax = uMax;
}
void RegulatorPID::setAntiWindup(double wlaczony)
{
    m_antiWindupWlaczony = wlaczony;
}

double RegulatorPID::algorytmP(double zadana, double regulowana)
{
    double uchyb = zadana - regulowana;
    m_uP = m_kp * uchyb;
    return m_uP;
}
double RegulatorPID::algorytmI(double zadana, double regulowana)
{
    double uchyb = zadana - regulowana;

    if(m_trybCalkowania == TrybCalkowania::STALA_PRZED_SUMA)
    {
        m_sumaUchybow += uchyb;
        m_uI = (m_Ti != 0.0) ? (m_sumaUchybow / m_Ti) : 0.0;
    }
    else
    {
        m_uI += (m_Ti != 0.0) ? (uchyb / m_Ti) : 0.0;
    }

    return m_uI;
}
double RegulatorPID::algorytmD(double zadana, double regulowana)
{
    double uchyb = zadana - regulowana;
    m_uD = m_Td * (uchyb - m_poprzedniUchyb);
    m_poprzedniUchyb = uchyb;
    return m_uD;
}
double RegulatorPID::Sterowanie(double zadana, double regulowana)
{
    if(m_uMin > m_uMax)
    {
        throw std::invalid_argument("Ograniczenie dolne nie może być większe od górnego!");
    }

    double uP = algorytmP(zadana, regulowana);
    double uI = algorytmI(zadana, regulowana);
    double uD = algorytmD(zadana, regulowana);
    double uchyb = zadana - regulowana;

    double U = uP + uI + uD;

    if (m_uMax != 0.0 || m_uMin != 0.0)
    {
        if (U > m_uMax)
        {
            U = m_uMax;

            if (m_antiWindupWlaczony)
            {
                if(uI > m_uMax && uchyb > 0)
                {
                    m_sumaUchybow -= uchyb;
                }

            }
        }
        else if (U < m_uMin)
        {
            U = m_uMin;

            if (m_antiWindupWlaczony)
            {
                if(uI < m_uMin && uchyb < 0)
                {
                    m_sumaUchybow -= uchyb;
                }
            }
        }
    }

    m_U = U;
    return m_U;
}

double RegulatorPID::getUP() {return m_uP;}
double RegulatorPID::getUI() {return m_uI;}
double RegulatorPID::getUD() {return m_uD;}
double RegulatorPID::getKp() {return m_kp;}
double RegulatorPID::getTi() {return m_Ti;}
double RegulatorPID::getTd() {return m_Td;}
