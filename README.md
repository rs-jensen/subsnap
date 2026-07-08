# subsnap

Automatically fixes subtitle sync in your media library. Detects how far off your subtitles are from the video and corrects them, including linear drift caused by framerate mismatches.

Built in C++ with FFmpeg for audio decoding and Python for orchestration. Stores what it has already processed in SQLite so it does not touch the same file twice.

---

## how it works

1. Decodes audio from the video file
2. Runs voice activity detection (VAD) to build a speech activity profile
3. Parses the SRT file and builds a dialogue activity profile from the timestamps
4. Cross-correlates the two profiles across multiple segments of the film to detect both constant offset and linear drift
5. Fits a line through the results and applies the correction to the SRT file
6. Keeps a .bak of the original just in case

---

## setup

### docker (recommended)

Copy the compose snippet and point it at your media:

```yaml
services:
  subsnap:
    image: ghcr.io/rs-jensen/subsnap:latest
    restart: always
    volumes:
      - ./data:/data
      - /your/movies:/media/movies
      - /your/series:/media/series
    environment:
      - MEDIA_ROOT=/media
      - DB_PATH=/data/subsnap.db
```

```bash
docker compose up -d
```

It will scan your library on startup, fix what it finds, then keep watching for new files.

### manual

Build the C++ binary:

```bash
g++ -o subsnap main.cpp decoder.cpp srt_parser.cpp correlate.cpp \
    $(pkg-config --cflags --libs libavformat libavcodec libavutil libswresample)
```

Install Python dependencies:

```bash
pip install watchdog
```

Run:

```bash
python3 main.py
```

Or run subsnap directly to see what offset it detects:

```bash
./subsnap video.mp4 subtitles.srt
```

This only prints the detected offset — it does not modify anything. Run `main.py` to actually correct the file.

---

## notes

subsnap matches video files to SRT files by filename similarity so it handles the usual naming chaos from scene releases reasonably well. If you have a very unusual setup it might match the wrong files, worth checking the first run.

The original SRT is always backed up as `.srt.bak` before anything is changed.

## supported formats

**Video:** `.mp4`, `.mkv`, `.avi`

**Subtitles:** `.srt`

Planned additions:
- Video: `.mov`, `.m4v`, `.wmv`, `.flv`, `.webm`, `.ts`
- Subtitles: `.ass`/`.ssa` (Advanced SubStation Alpha), `.vtt` (WebVTT), `.sub` (MicroDVD)