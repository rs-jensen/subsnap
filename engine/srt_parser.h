#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

std::vector<std::pair<int, int>> read_srt(const char* filename);
std::vector<int> activity(std::vector<std::pair<int, int>> timestamps);
void write_srt(const char* input_path, const char* output_path, double slope, double intercept_s);