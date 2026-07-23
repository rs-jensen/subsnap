#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fvad.h>


extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
    #include <libswresample/swresample.h>
}

AVFormatContext* open_file(const char* filename);
int find_audio_stream(const AVFormatContext* pFormatContext);
AVCodecContext* open_audio_decoder(const AVFormatContext* pFormatContext, int audio_stream_index);
std::vector<float> decode_audio(AVFormatContext* pFormatContext, AVCodecContext* dec_ctx, int audio_stream_index);
std::vector<int16_t> convert_to_8kHz(std::vector<float> decoded, AVCodecContext* dec_ctx);
std::vector<float> calculate_fvad(const std::vector<int16_t>& pcm, int sample_rate);
