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
    std::cout << "Sample fmt: " << OAD->sample_fmt << '\n';
    std::cout << "Channels: " << OAD->ch_layout.nb_channels << '\n';
    std::vector<float> decoded = decode_audio(AVC, OAD, audio_stream_index);
    std::vector<int16_t> pcm = convert_to_8kHz(decoded, OAD);
    std::cout << "pcm.size(): " << pcm.size() << '\n';
    std::cout << "expected: " << decoded.size() / 6 << '\n'; // 48000/8000 = 6
    std::cout << "first samples: ";
    for (int i = 0; i < 10; ++i)
        std::cout << pcm[i] << " ";
    std::cout << '\n';
    std::vector<float> fvad = calculate_fvad(pcm, 8000);

    std::vector<std::pair<int, int>> timestamps = read_srt(argv[2]);
    std::vector<int> activity_variable = activity(timestamps);
    std::vector<int> fvad_int(fvad.begin(), fvad.end());
    auto [slope, intercept] = fft_crosscorrelate(fvad_int, activity_variable);

    std::cout << "Samples: " << decoded.size() << '\n';
    std::cout << "Sample rate: " << OAD->sample_rate << '\n';
    std::cout << "fvad.size(): " << fvad.size() << '\n';
    std::cout << "activity_variable.size: " << activity_variable.size() << '\n';
    std::cout << "Timestamps size: " << timestamps.size() << '\n';
    std::cout << "Endtime: " << timestamps.back().second << '\n';

    return 0;
}
