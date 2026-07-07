#include "correlate.h"

// Finds the offset between the movie and subtitles
std::pair<int, float> cross_correlation(std::vector<int> activity_profile, std::vector<float> RMS) {
    // starts checking one minute behind
    float best_sum = -std::numeric_limits<float>::infinity();
    float second_best_sum = -std::numeric_limits<float>::infinity();
    int initial_offset {};
    int offset_other {};
    float all_sums = 0;
    float all_sums_other = 0;
    int count = 0;
    int chunk_number = 0;
    long long activity = 0;
    for (int  chunks = 45000; chunks + 90000 <= activity_profile.size(); chunks += 90000) {
        chunk_number += 1;
        activity = 0;
        for (int i = chunks; i < chunks + 90000; ++i)
            activity += activity_profile[i];

        if (chunk_number == 1) {
            for (int offset = -120000; offset <= 120000; offset += 10) {
                float sum = 0;
                int offset_index = offset / 10;

                // The idea is: slide the subtitle timeline over the audio energy profile, multiply
                // and sum at each offset. Loud audio + subtitle activity aligned = high score.
                // The offset with the highest score is the sync point.

                for (int i = 0; i < 90000; ++i) {
                    int rms_index = i - offset_index;
                    if (rms_index >= 0 && rms_index < RMS.size()) {
                    sum += activity_profile[i] * RMS[rms_index];
                    }
                }
                if (sum > best_sum) {
                    best_sum = sum;
                    initial_offset = offset;
                } else if (abs(offset - initial_offset) > 1000 && sum > second_best_sum) {
                    second_best_sum = sum;
                }
                all_sums += sum;
                count++;
                //if (offset <= -120000 + 200)
                //    std::cout << "offset: " << offset << " sum: " << sum << " best: " << best_sum << '\n';
            }
        std::cout << "t_" << chunk_number << " " << initial_offset << '\n';
        std::cout << "Activity: " << activity << '\n';
        float sharpness = best_sum / second_best_sum;
        std::cout << "Sharpness_" << chunk_number << ": " << sharpness << '\n';
        } else {
            best_sum = -std::numeric_limits<float>::infinity();
            second_best_sum = -std::numeric_limits<float>::infinity();
            offset_other = 0;

            for (int offset = -10000; offset <= 10000; offset += 10) {
                float sum = 0;
                int offset_index = offset / 10; 
                
                for (int i = chunks; i < chunks + 90000; ++i) {
                    int rms_index = i - offset_index;
                    if (rms_index >= 0 && rms_index < RMS.size()) {
                    sum += activity_profile[i] * RMS[rms_index];
                    }
                }
                if (sum > best_sum) {
                    best_sum = sum;
                    offset_other = offset;
                } else if (abs(offset - offset_other) > 1000 && sum > second_best_sum) {
                    second_best_sum = sum;
                }
            }
        std::cout << "t_" << chunk_number << " " << offset_other << '\n';
        std::cout << "Activity_" << chunk_number << ": " << activity << '\n';
        float sharpness = best_sum / second_best_sum;
        std::cout << "Sharpness_" << chunk_number << ": " << sharpness << '\n';
        }
    }


    float average_sum = all_sums / count;
    float confidence = best_sum / average_sum;
    return {initial_offset, confidence};
}