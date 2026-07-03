FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    g++ \
    pkg-config \
    libavformat-dev \
    libavcodec-dev \
    libavutil-dev \
    libswresample-dev \
    libfftw3-dev \
    python3 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY *.cpp *.h main.py ./

RUN g++ -o subsnap main.cpp decoder.cpp srt_parser.cpp correlate.cpp \
    $(pkg-config --cflags --libs libavformat libavcodec libavutil libswresample) \
    -lfftw3

CMD ["python3", "main.py"]
