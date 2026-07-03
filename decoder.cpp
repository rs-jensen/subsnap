#include "decoder.h"

// Opens the file, allocate memory, finds stream info. If returns nullptr cause the function is pointer
AVFormatContext* open_file(const char* filename) {
        AVFormatContext *pFormatContext = avformat_alloc_context();
    if (!pFormatContext) {
        std::cerr << "ERROR could not allocate memory for Format Context" << '\n';
        return nullptr;
    }

    if (avformat_open_input(&pFormatContext, filename, NULL, NULL) != 0) {
        std::cerr << "ERROR could not open the file" << '\n';
        return nullptr;
    }
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
        std::cerr << "ERROR could not get stream info" << '\n';
        return nullptr;
    }
    
    std::cout << "Format: " << pFormatContext->iformat->name << " Duration: " << pFormatContext->duration << '\n';
    return pFormatContext;
}

// Find the audio by looping though the streams from pFormatContext then checking by matching with the media type
int find_audio_stream(const AVFormatContext* pFormatContext) {
    for(int i {0}; i < pFormatContext->nb_streams; ++i) {
        if (pFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            // std::cout << i << '\n';
            return i;
        }

    }    
    return -1;
}

// This is supposed to decode and open the audio from find_audio_stream()
AVCodecContext* open_audio_decoder(const AVFormatContext* pFormatContext, int audio_stream_index) {
    // Casually checking if we can open the decoder ))
    if (audio_stream_index <0) {
        std::cerr << "Error opening the decoder: " << '\n';
        return nullptr;
    }

    // Look for the same codec de decode
    const AVCodec *decoder = avcodec_find_decoder(pFormatContext->streams[audio_stream_index]->codecpar->codec_id);

    if (!decoder) {
        std::cerr << "Necessary decoder not found" << '\n';
        return nullptr;
    }
    AVCodecContext *dec_ctx = avcodec_alloc_context3(decoder);
    if (!dec_ctx) {
        std::cerr << "Failed to allocate decoder context" << '\n';
    }
    // This to get value from AVCodecParameters to the AVCodecContext so avcodec_open2 can work with it
    avcodec_parameters_to_context(dec_ctx, pFormatContext->streams[audio_stream_index]->codecpar);

    int ret = avcodec_open2(dec_ctx, NULL, NULL);
    if (ret < 0) {
        std::cerr << "Cannot open audio decoder" << '\n';
        return nullptr;
    }
    return dec_ctx;
}

// Decode the audio! My idea with vector float is that every float is a PCM-sample and a vector will grow while it reads packages from the file 
std::vector<float> decode_audio(AVFormatContext* pFormatContext, AVCodecContext* dec_ctx ,int audio_stream_index) {
    int ret;
    AVPacket *packet = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();

    std::vector<float> decoded = {};

    if (!packet || !frame) {
        std::cerr << "Cloud not allocate frame or packet" << '\n';
        return {};
    }

    // Read all the packets
    while (1) {
        // Reads the compressed package from  file and puts it in a packet. 
        // Returns negative when the file is done
        if ((ret = av_read_frame(pFormatContext, packet)) < 0) {
            return decoded;
        }
        // Filter so we only get audio streams
        if (packet->stream_index == audio_stream_index) {
            // Send the stuff to the decoder
            ret = avcodec_send_packet(dec_ctx, packet);
            if (ret < 0) {
                std::cerr << "Error while sending a packet to the decoder" << '\n';
                return decoded;
            }
            // This loop is so we can get the decoded frames. The loop is cause i tested without and i didnt work so a packet can have multiple frames-
            while (ret >= 0) {
                // Retrieves one decoded frame "EAGAIN" basically means not ready yet send more packages.
                ret = avcodec_receive_frame(dec_ctx, frame);

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                } else if (ret < 0) {
                    std::cerr << "Error while receiving a frame from the decoder" << '\n';
                    return decoded;
                }
                // The line below basically is just telling it to treat these bytes as another type here its "the data in raw bytes in frame->data[0] is actually floats"
                // Or i hope so at least it works ))
                float* samples = reinterpret_cast<float*>(frame->data[0]);
                for (int i = 0;i < frame->nb_samples ; ++i) {
                    decoded.push_back(samples[i]);
                }
            }
        }
    }
    return decoded; 
}


//  trying FFTW3 instead think thats better instead of calculate_RMS sometime !!


// Calculate the RMS (Root mean square) and find the sound volume in 10 ms windows.
// The sample rate is 44100 so 44100/100 = 441 samples/10ms
// Had to cahnge to window size idk think its cause it dosnt match the activity profiles timescale so just trying this.

std::vector<float> calculate_RMS(const std::vector<float>& decoded, int sample_rate) {
    std::vector<float> RMS = {};
    int window_size = sample_rate / 100;
    float sum = 0;

    // RMS = sqrt(( x1^2+x2^2...xn^2) / n)
    for (int i = 0;i < decoded.size(); ++i) {
        float j = decoded[i];
        j *= j;
        sum += j;
        
        if (i % window_size == window_size - 1) {
            sum /= window_size;
            sum = sqrt(sum);
            //std::cout << "Sum: " << sum << '\n';
            RMS.push_back(sum);
            sum = 0;
        }
    }
    return RMS;
}
