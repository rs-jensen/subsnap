import os
import sqlite3
import subprocess

DB_PATH = os.environ.get("DB_PATH", "subsnap.db")

def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS sync_jobs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            video_path TEXT NOT NULL,
            srt_path TEXT NOT NULL,
            offset_ms INTEGER,
            status TEXT DEFAULT 'pending',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.commit()
    return conn

def find_pairs(path):
    videos = {}
    subtitles = {}

    for root, dirs, files in os.walk(path):
        for filename in files:
            name = os.path.splitext(filename)[0]
            full_path = os.path.join(root, filename)

            if filename.endswith(('.mp4', '.mkv', '.avi')):
                videos[name] = full_path
            elif filename.endswith('.srt'):
                subtitles[name] = full_path

    pairs = []
    for name, video in videos.items():
        if name in subtitles:
            pairs.append((video, subtitles[name]))

    return pairs

def run_sync(video_path, srt_path):
    result = subprocess.run(
        ["./subsnap", video_path, srt_path],
        capture_output=True,
        text=True
    )
    for line in result.stdout.splitlines():
        if line.startswith("Offset in ms:"):
            return int(line.split(":")[1].strip())
    return None

def save_result(conn, video_path, srt_path, offset_ms):
    conn.execute(
        "INSERT INTO sync_jobs (video_path, srt_path, offset_ms, status) VALUES (?, ?, ?, ?)",
        (video_path, srt_path, offset_ms, "done")
    )
    conn.commit()

def main():
    conn = init_db()
    pairs = find_pairs("/")

    for video, srt in pairs:
        print(f"Processing: {video}")
        offset = run_sync(video, srt)
        if offset is not None:
            print(f"Offset: {offset}ms")
            save_result(conn, video, srt, offset)
        else:
            print(f"Failed: {video}")

    conn.close()

if __name__ == "__main__":
    main()
