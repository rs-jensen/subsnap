#pragma once
#include <vector>
#include <iostream>
#include <limits>
#include <tuple>
#include <utility>

std::pair<double, double> linear_regression(std::vector<double> x, std::vector<double> y, std::vector<double> w);
std::tuple<double, double, float> cross_correlation(std::vector<int> activity_profile, std::vector<float> RMS);
