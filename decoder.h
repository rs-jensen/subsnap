#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fftw3.h>

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

AVFormatContext* open_file(const char* filename);
int find_audio_stream(const AVFormatContext* pFormatContext);
AVCodecContext* open_audio_decoder(const AVFormatContext* pFormatContext, int audio_stream_index);
std::vector<float> decode_audio(AVFormatContext* pFormatContext, AVCodecContext* dec_ctx ,int audio_stream_index);
std::vector<float> calculate_RMS(const std::vector<float>& decoded, int sample_rate);
