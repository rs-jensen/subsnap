#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include "decoder.h"
#include "srt_parser.h"
#include <algorithm>
#include "correlate.h"

int main(int argc, const char *argv[]) {
    if (argc < 3) {
        std::cout << "You need to specify a media file and a srt file" << '\n';
        return -1;
    }
    AVFormatContext* AVC = open_file(argv[1]);
    int audio_stream_index = find_audio_stream(AVC);
    AVCodecContext* OAD = open_audio_decoder(AVC, audio_stream_index);
    std::vector<float> decoded = decode_audio(AVC, OAD, audio_stream_index);
    std::vector<float> fvad = calculate_fvad(decoded, OAD->sample_rate);
    float max_val = *std::max_element(fvad.begin(), fvad.end());

    //for (int i = 351000; i < 352000; ++i)
    //    std::cout << "FFT[" << i << "]: " << FFT[i] << '\n';

    std::vector<std::pair<int, int>> timestamps = read_srt(argv[2]);
    std::vector<int> activity_variable = activity(timestamps);
    auto [slope, intercept, confidence] = cross_correlation(activity_variable, fvad);

    
    //for (int i = 0; i < 10; ++i)
    //    std::cout << "RMS[" << i << "]: " << RMS[i] << " activity[" << i << "]: " << activity_variable[i] << '\n';

    std::cout << "Samples: " << decoded.size() << '\n';
    std::cout << "Sample rate: " << OAD->sample_rate << '\n';
    std::cout << "RMS.size(): " << fvad.size() << '\n';
    std::cout << "activity_variable.size: " << activity_variable.size() << '\n';
    std::cout << "Timestamps size: " << timestamps.size() << '\n';
    std::cout << "Endtime: " << timestamps.back().second << '\n';
    /*
    for (int i = 0; i < 10; ++i) {
        int j = timestamps[i];
        std::cout << "Timestamp: " << j << '\n';
    }
    */
    return 0;
}
