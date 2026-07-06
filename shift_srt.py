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

def shift_timestamp(ts: str, offset_ms: int) -> str:
    hh = int(ts[0:2])
    mm = int(ts[3:5])
    ss = int(ts[6:8])
    ms = int(ts[9:12])

    total_ms = (((hh * 60 + mm) * 60 + ss) * 1000) + ms + offset_ms
    if total_ms < 0:
        total_ms = 0

    hh = total_ms // 3600000
    total_ms %= 3600000
    mm = total_ms // 60000
    total_ms %= 60000
    ss = total_ms // 1000
    ms = total_ms % 1000

    return f"{hh:02}:{mm:02}:{ss:02},{ms:03}"

def shift_line(line: str, offset_ms: int) -> str:
    if " --> " not in line:
        return line

    parts = line.split(" --> ")
    if len(parts) != 2:
        return line

    start = parts[0].strip()
    end = parts[1].strip()

    if len(start) != 12 or len(end) != 12:
        return line

    try:
        new_start = shift_timestamp(start, offset_ms)
        new_end = shift_timestamp(end, offset_ms)
        return f"{new_start} --> {new_end}"
    except Exception:
        return line

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 shift_srt.py input.srt output.srt offset_ms")
        sys.exit(1)

    input_file = Path(sys.argv[1])
    output_file = Path(sys.argv[2])
    offset_ms = int(sys.argv[3])

    raw = input_file.read_bytes()
    encoding = detect_encoding(raw)
    text = raw.decode(encoding, errors="replace")

    out_lines = []
    for line in text.splitlines():
        out_lines.append(shift_line(line, offset_ms))

    output_file.write_text("\n".join(out_lines) + "\n", encoding="utf-8")
    print(f"Shifted {input_file} by {offset_ms} ms -> {output_file}")

if __name__ == "__main__":
    main()