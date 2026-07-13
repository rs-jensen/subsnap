FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    python3 python3-pip \
    ffmpeg \
    libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
    libfftw3-dev \
    g++ pkg-config \
    git automake autoconf libtool \
    && rm -rf /var/lib/apt/lists/*

RUN git clone https://github.com/dpirch/libfvad.git /tmp/libfvad && \
    cd /tmp/libfvad && \
    autoreconf -i && \
    ./configure && \
    make && \
    make install && \
    ldconfig && \
    rm -rf /tmp/libfvad
    
WORKDIR /app
COPY requirements.txt .
RUN pip3 install -r requirements.txt --break-system-packages
COPY correlate.cpp correlate.h decoder.cpp decoder.h srt_parser.cpp srt_parser.h main.cpp .
RUN g++ -O2 -o subsnap main.cpp correlate.cpp decoder.cpp srt_parser.cpp \
    -lavcodec -lavformat -lavutil -lswresample -lfvad -lfftw3
    
COPY main.py shift_srt.py .
CMD ["python3", "main.py"]
