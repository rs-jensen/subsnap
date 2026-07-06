#pragma once
#include <vector>
#include <iostream>
#include <limits>

std::pair<int, float> cross_correlation(std::vector<int> activity_profile, std::vector<float> RMS);
