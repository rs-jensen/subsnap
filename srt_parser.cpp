#include "srt_parser.h"

std::vector<std::pair<int, int>> read_srt(const char* filename) {
    std::vector<std::pair<int, int>> timestamps;
    std::string line {};
    std::ifstream read_file(filename);
    while (getline (read_file, line)) {
        if (line.find("-->") != std::string::npos) {
            int start_ms {};
            int end_ms {};

            int hours =std::stoi(line.substr(0, 2));
            start_ms = hours * 3600000;
            int min =std::stoi(line.substr(3, 2));
            start_ms += min * 60000;
            int sec =std::stoi(line.substr(6, 2));
            start_ms += sec * 1000;
            int mil_sec =std::stoi(line.substr(9, 3));
            start_ms += mil_sec;

            int e_hours =std::stoi(line.substr(17, 2));
            end_ms = e_hours * 3600000;
            int e_min =std::stoi(line.substr(20, 2));
            end_ms += e_min * 60000;
            int e_sec =std::stoi(line.substr(23, 2));
            end_ms += e_sec * 1000;
            int e_mil_sec =std::stoi(line.substr(26, 3));
            end_ms += e_mil_sec;

            timestamps.push_back(std::make_pair(start_ms, end_ms));
        }
    }
    return timestamps;
}

// Build some sort of activity profile that checks every 10ms for dialogue 
std::vector<int> activity(std::vector<std::pair<int, int>> timestamps) {
    std::vector<int> activity_profile = {};
    int j = 0;
    for (int i = 0; i <= timestamps.back().second; i += 10) {
        while (j < timestamps.size() && i > timestamps[j].second) {
            j++;
        }
        if (i >= timestamps[j].first && i <= timestamps[j].second) {
            activity_profile.push_back(1);
        } else {
            activity_profile.push_back(0);
        }
    }
    return activity_profile;
}
