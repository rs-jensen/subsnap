import os
import sqlite3
import subprocess
import shutil
import time
import numpy as np
from pathlib import Path
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

DB_PATH    = os.environ.get("DB_PATH", "subsnap.db")
MEDIA_ROOT = os.environ.get("MEDIA_ROOT", "/media")
SUBSNAP    = os.environ.get("SUBSNAP_BIN", "./subsnap")
CHUNK_DUR  = 900
VIDEO_EXTS = ('.mp4', '.mkv', '.avi')


def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS sync_jobs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            video_path TEXT NOT NULL,
            srt_path TEXT NOT NULL,
            backup_path TEXT,
            slope REAL,
            intercept_ms REAL,
            status TEXT DEFAULT 'pending',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.execute("""
        CREATE UNIQUE INDEX IF NOT EXISTS idx_sync_jobs_video_srt
        ON sync_jobs(video_path, srt_path)
    """)
    for col in [("slope", "REAL"), ("intercept_ms", "REAL")]:
        try:
            conn.execute(f"ALTER TABLE sync_jobs ADD COLUMN {col[0]} {col[1]}")
        except:
            pass
    conn.commit()
    return conn


def normalize(name):
    for ch in "._-":
        name = name.replace(ch, " ")
    return set(name.lower().split())


def find_pairs(path):
    videos = []
    subtitles = []

    for root, dirs, files in os.walk(path):
        for filename in files:
            full_path = os.path.join(root, filename)
            base = os.path.splitext(filename)[0]
            if filename.lower().endswith(VIDEO_EXTS):
                videos.append((base, full_path))
            elif filename.lower().endswith('.srt'):
                subtitles.append((base, full_path))

    pairs = []
    for vbase, vpath in videos:
        best, best_score = None, 0
        for sbase, spath in subtitles:
            score = len(normalize(vbase) & normalize(sbase))
            if score > best_score:
                best_score = score
                best = spath
        if best and best_score >= 3:
            print(f"Matched: {vpath} -> {best}")
            pairs.append((vpath, best))
        else:
            print(f"No srt match for: {vpath}")

    return pairs


def already_processed(conn, video_path, srt_path):
    cur = conn.execute(
        "SELECT 1 FROM sync_jobs WHERE video_path = ? AND srt_path = ? AND status = 'done' LIMIT 1",
        (video_path, srt_path)
    )
    return cur.fetchone() is not None


def run_sync(video_path, srt_path):
    result = subprocess.run(
        [SUBSNAP, video_path, srt_path],
        capture_output=True,
        text=True,
        check=True
    )

    chunks = {}
    current = None
    for line in result.stdout.splitlines():
        if line.startswith("t_"):
            parts = line.split()
            current = int(parts[0][2:])
            chunks[current] = {"offset": int(parts[1])}
        elif line.startswith("Activity") and current is not None:
            chunks[current]["activity"] = float(line.split()[-1])
        elif line.startswith("Sharpness") and current is not None:
            chunks[current]["sharpness"] = float(line.split()[-1])

    t_vals, delta_vals, weights = [], [], []
    for n, data in chunks.items():
        sharpness = data.get("sharpness", 0)
        if sharpness < 1.01:
            continue
        t_vals.append((n - 0.5) * CHUNK_DUR)
        delta_vals.append(data["offset"] / 1000.0)
        weights.append(sharpness)

    if len(t_vals) < 2:
        raise RuntimeError("Not enough reliable chunks")

    slope, intercept = np.polyfit(t_vals, delta_vals, 1, w=weights)
    print(f"Slope: {slope:.6f}  Intercept: {intercept:.3f}s")
    return slope, intercept


def backup_and_shift_srt(srt_path, slope, intercept):
    srt = Path(srt_path)
    backup_path = srt.with_suffix(".srt.bak")
    shutil.copy2(srt, backup_path)

    subprocess.run(
        ["python3", "shift_srt.py", str(srt), str(srt), str(-slope), str(-intercept)],
        check=True
    )
    return str(backup_path)


def save_result(conn, video_path, srt_path, backup_path, slope, intercept, status="done"):
    intercept_ms = intercept * 1000 if intercept is not None else None
    conn.execute(
        """
        INSERT OR REPLACE INTO sync_jobs
        (video_path, srt_path, backup_path, slope, intercept_ms, status)
        VALUES (?, ?, ?, ?, ?, ?)
        """,
        (video_path, srt_path, backup_path, slope, intercept_ms, status)
    )
    conn.commit()


def process(conn, video_path, srt_path):
    if already_processed(conn, video_path, srt_path):
        print(f"Skipping: {video_path}")
        return

    print(f"Processing: {video_path}")
    try:
        slope, intercept = run_sync(video_path, srt_path)
        backup_path = backup_and_shift_srt(srt_path, slope, intercept)
        save_result(conn, video_path, srt_path, backup_path, slope, intercept)
        print(f"Done: {video_path}")
    except Exception as e:
        print(f"Failed: {video_path} -> {e}")
        save_result(conn, video_path, srt_path, None, None, None, status="failed")


class MediaHandler(FileSystemEventHandler):
    def __init__(self, conn):
        self.conn = conn

    def on_created(self, event):
        if event.is_directory:
            return
        time.sleep(5)
        pairs = find_pairs(os.path.dirname(event.src_path))
        for video, srt in pairs:
            process(self.conn, video, srt)


def main():
    conn = init_db()

    for video, srt in find_pairs(MEDIA_ROOT):
        process(conn, video, srt)

    print("Watching for new files...")
    handler = MediaHandler(conn)
    observer = Observer()
    observer.schedule(handler, MEDIA_ROOT, recursive=True)
    observer.start()

    try:
        while True:
            time.sleep(10)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()
    conn.close()


if __name__ == "__main__":
    main()