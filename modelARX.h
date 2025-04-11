#pragma once
#include <vector>
#include <deque>
#include <random>
#include <numeric>

class ModelARX {
private:
    std::vector<double> a;
    std::vector<double> b;
    std::deque<double> u;
    std::deque<double> y;
    int opoznienie;
    double odchStan;
    std::default_random_engine generator;
    std::normal_distribution<double> zaklocenie;
public:
    ModelARX(const std::vector<double>& a, const std::vector<double>& b, int opoznienie, double odchS);
    void setParametryARX(const std::vector<double>& a, const std::vector<double>& b, int opoznienie, double odchS);
    double obliczARX(double sygnalWej);
    void reset();
};
