# subsnap

**v1.0 — don't use this in production yet.** The current sync detection uses raw RMS energy which is pretty naive and often wrong. Plan is to replace it with FFT-based VAD (voice activity detection) filtering to 300-3400Hz so it can actually tell the difference between someone talking and an explosion. That's v2.

---

subsnap is a CLI tool that figures out how far off your subtitle file is from your video and tells you the offset in milliseconds. Built in C++ with FFmpeg for audio decoding and Python for orchestration and SQLite tracking.

## how it works

1. Decodes the audio from your video file
2. Calculates RMS energy in 10ms windows to build an audio activity profile
3. Parses the SRT file and builds a dialogue activity profile from the timestamps
4. Cross-correlates the two profiles to find the best matching offset
5. Prints the offset in ms

Right now subsnap only detects the offset — it does not modify the SRT file yet. That's coming in v2.

## usage

```bash
./subsnap video.mp4 subtitles.srt
```

Or run the Python orchestrator which scans your media library, runs subsnap on each video/subtitle pair, and stores results in SQLite:

```bash
python3 main.py
```

## docker

```bash
docker compose up
```

Mount your media directory in `docker-compose.yml`:

```yaml
volumes:
  - /your/media:/media
```

## building

```bash
g++ -o subsnap main.cpp decoder.cpp srt_parser.cpp correlate.cpp \
    $(pkg-config --cflags --libs libavformat libavcodec libavutil libswresample) \
    -lfftw3
```

## what's broken / what's next

- RMS-based sync detection works okay for clean audio but gets confused by music and sound effects
- v2 will use FFTW3 to filter to the speech frequency range (300-3400Hz) before correlation
- v2 will actually apply the offset and write a corrected SRT file
- v2 will watch for new files and automatically process them, with SQLite tracking what's already been fixed
- No drift correction yet (for when video and subtitles have different framerates)
