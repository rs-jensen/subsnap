#include "correlate.h"

// Finds the offset between the movie and subtitles
int cross_correlation(std::vector<int> activity_profile, std::vector<float> RMS) {
    // starts checking one minute behind
    float best_sum = -std::numeric_limits<float>::infinity();    
    int best_offset {};

    for (int offset = -120000; offset <= 120000; offset += 10) {
        float sum = 0;
        int offset_index = offset / 10;

        // The idea is: slide the subtitle timeline over the audio energy profile, multiply
        // and sum at each offset. Loud audio + subtitle activity aligned = high score.
        // The offset with the highest score is the sync point.

        for (int i = 0; i < activity_profile.size(); ++i) {
            int rms_index = i - offset_index;
            if (rms_index >= 0 && rms_index < RMS.size()) {
            sum += activity_profile[i] * RMS[rms_index];
                }
        }
        if (sum > best_sum) {
        best_sum = sum;
        best_offset = offset;
        }
        if (offset <= -120000 + 200)
            std::cout << "offset: " << offset << " sum: " << sum << " best: " << best_sum << '\n';
    }
    return best_offset;
}
