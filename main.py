import os
import sqlite3
import subprocess
import shutil
import re
from pathlib import Path


DB_PATH = os.environ.get("DB_PATH", "subsnap.db")


def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS sync_jobs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            video_path TEXT NOT NULL,
            srt_path TEXT NOT NULL,
            backup_path TEXT,
            offset_ms INTEGER,
            status TEXT DEFAULT 'pending',
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    """)
    conn.execute("""
        CREATE UNIQUE INDEX IF NOT EXISTS idx_sync_jobs_video_srt
        ON sync_jobs(video_path, srt_path)
    """)
    conn.commit()
    return conn


def find_pairs(path):
    videos = []
    subtitles = []

    for root, dirs, files in os.walk(path):
        for filename in files:
            full_path = os.path.join(root, filename)
            base = os.path.splitext(filename)[0]

            if filename.lower().endswith(('.mp4', '.mkv', '.avi')):
                videos.append((base, full_path))
            elif filename.lower().endswith('.srt'):
                subtitles.append((base, full_path))

    pairs = []

    for vbase, vpath in videos:
        matches = []

        for sbase, spath in subtitles:
            if sbase == vbase or sbase.startswith(vbase + "."):
                matches.append(spath)

        if matches:

            pairs.append((vpath, matches[0]))
            print(f"Matched: {vpath} -> {matches[0]}")
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
        ["./subsnap", video_path, srt_path],
        
        capture_output=True,
        text=True,
        check=True
    )

    for line in result.stdout.splitlines():
        if line.startswith("Offset in ms:"):
            
            return int(line.split(":", 1)[1].strip())



    raise RuntimeError("Could not parse offset from subsnap output")


def backup_and_shift_srt(srt_path, offset_ms):
    srt = Path(srt_path)
    backup_path = srt.with_suffix(".srt.bak")
    shutil.copy2(srt, backup_path)
    correction_ms = -offset_ms

    subprocess.run(
        ["python3", "shift_srt.py", str(srt), str(srt), str(correction_ms)],
        check=True
    )
    return str(backup_path)



def save_result(conn, video_path, srt_path, backup_path, offset_ms):
    conn.execute(
        """
        INSERT OR REPLACE INTO sync_jobs
        (video_path, srt_path, backup_path, offset_ms, status)
        VALUES (?, ?, ?, ?, ?)
        """,
        (video_path, srt_path, backup_path, offset_ms, "done")
    )
    conn.commit()


def main():
    conn = init_db()
    pairs = find_pairs("/")

    for video, srt in pairs:
        if already_processed(conn, video, srt):
            print(f"Skipping already processed: {video}")
            continue

        print(f"Processing: {video}")
        try:
            offset = run_sync(video, srt)
            print(f"Offset: {offset}ms")

            backup_path = backup_and_shift_srt(srt, offset)
            print(f"Backup saved: {backup_path}")

            save_result(conn, video, srt, backup_path, offset)
            print(f"Done: {video}")
        except Exception as e:
            print(f"Failed: {video} -> {e}")
            conn.execute(
                """
                INSERT OR REPLACE INTO sync_jobs
                (video_path, srt_path, offset_ms, status)
                VALUES (?, ?, ?, ?)
                """,
                (video, srt, None, "failed")
            )
            conn.commit()

    conn.close()


if __name__ == "__main__":
    main()