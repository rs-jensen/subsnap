FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y \
    python3 python3-pip \
    ffmpeg \
    libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
    g++ pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY requirements.txt .
RUN pip3 install -r requirements.txt --break-system-packages

COPY src/ ./src/
RUN g++ -O2 -o subsnap src/correlate.cpp src/decoder.cpp src/srt_parser.cpp \
    $(pkg-config --cflags --libs libavcodec libavformat libavutil libswresample)

COPY main.py shift_srt.py .

CMD ["python3", "main.py"]
