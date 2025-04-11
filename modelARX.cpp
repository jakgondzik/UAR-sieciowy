#include "modelARX.h"
#include <QDebug>

ModelARX::ModelARX(const std::vector<double>& wspa, const std::vector<double>& wspb, int opoznienieK, double odchStan)
    :a(wspa), b(wspb), u(opoznienieK + 10 + b.size(), 0.0), y(a.size() + 10, 0.0), opoznienie(opoznienieK), zaklocenie(0.0, odchStan)
{}

void ModelARX::setParametryARX(const std::vector<double>& wspa, const std::vector<double>& wspb, int opoznienieK, double odchS)
{
    a = wspa;
    b = wspb;
    opoznienie = opoznienieK;
    odchStan = (odchS > 0.0) ? odchS : 0.000000000001;

    zaklocenie = std::normal_distribution<double>(0.0, odchStan);

    if (u.size() <= opoznienie) {
        while (u.size() <= opoznienie) {
            u.push_front(0.0);
        }
    }


    if (y.size() <= a.size()) {
        while (y.size() <= a.size()) {
            y.push_front(0.0);
        }
    }

}


double ModelARX::obliczARX(double sygnalWej)
{
    if (u.size() > 0)
        u.pop_back();


    u.push_front(sygnalWej);


    double wyjscie = std::inner_product(b.begin(), b.end(), u.begin() + opoznienie, 0.0) - std::inner_product(a.begin(), a.end(), y.begin(), 0.0);

    static std::default_random_engine generator(std::random_device{}());
    double z = zaklocenie(generator);

    wyjscie = wyjscie + z;

    if (y.size() > 0)
        y.pop_back();

    y.push_front(wyjscie);

    return y.front();


}


void ModelARX::reset()
{
    std::fill(u.begin(), u.end(), 0.0);
    std::fill(y.begin(), y.end(), 0.0);

}
