FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    python3 python3-pip \
    ffmpeg \
    libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
    libfvad-dev \
    g++ pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt .
RUN pip3 install -r requirements.txt --break-system-packages

COPY correlate.cpp correlate.h decoder.cpp decoder.h srt_parser.cpp srt_parser.h main.cpp .
RUN g++ -O2 -o subsnap main.cpp correlate.cpp decoder.cpp srt_parser.cpp \
    -lavcodec -lavformat -lavutil -lswresample -lfvad

COPY main.py shift_srt.py .

CMD ["python3", "main.py"]
