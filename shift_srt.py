import sys
from pathlib import Path


def detect_encoding(data: bytes) -> str:
    for enc in ("utf-8-sig", "utf-8", "cp1252", "latin-1"):
        try:
            data.decode(enc)
            return enc
        except UnicodeDecodeError:
            pass
    return "latin-1"


def ts_to_seconds(ts: str) -> float:
    hh, mm, rest = ts.split(":")
    ss, ms = rest.split(",")
    return int(hh) * 3600 + int(mm) * 60 + int(ss) + int(ms) / 1000.0


def seconds_to_ts(t: float) -> str:
    if t < 0:
        t = 0
    ms = round((t % 1) * 1000)
    if ms == 1000:
        ms = 0
        t += 1
    t = int(t)
    return f"{t // 3600:02}:{(t % 3600) // 60:02}:{t % 60:02},{ms:03}"


def correct_line(line: str, slope: float, intercept: float) -> str:
    if " --> " not in line:
        return line
    parts = line.split(" --> ")
    if len(parts) != 2:
        return line
    try:
        start = ts_to_seconds(parts[0].strip())
        end = ts_to_seconds(parts[1].strip())
        # linear correction: t_new = t_old * (1 + slope) + intercept
        new_start = seconds_to_ts(start * (1 + slope) + intercept)
        new_end = seconds_to_ts(end * (1 + slope) + intercept)
        return f"{new_start} --> {new_end}"
    except Exception:
        return line


def main():
    if len(sys.argv) != 5:
        print("Usage: shift_srt.py input.srt output.srt slope intercept_seconds")
        sys.exit(1)

    input_file = Path(sys.argv[1])
    output_file = Path(sys.argv[2])
    slope = float(sys.argv[3])
    intercept = float(sys.argv[4])

    raw = input_file.read_bytes()
    enc = detect_encoding(raw)
    lines = raw.decode(enc, errors="replace").splitlines()

    corrected = [correct_line(l, slope, intercept) for l in lines]
    output_file.write_text("\n".join(corrected) + "\n", encoding="utf-8")
    print(f"Done: slope={slope:.6f} intercept={intercept:.3f}s -> {output_file}")


if __name__ == "__main__":
    main()