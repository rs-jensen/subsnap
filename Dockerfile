FROM debian:bookworm-slim
 
RUN apt-get update && apt-get install -y \
    python3 python3-pip \
    ffmpeg \
    libavcodec-dev libavformat-dev libavutil-dev libswresample-dev \
    && rm -rf /var/lib/apt/lists/*
 
WORKDIR /app
 
COPY requirements.txt .
RUN pip3 install -r requirements.txt --break-system-packages
 
COPY subsnap .
COPY main.py shift_srt.py .
 
CMD ["python3", "main.py"]