import csv
import os
import statistics
import subprocess
import time
from pathlib import Path


"""Пути"""
SCRIPT_DIR  = Path(__file__).resolve().parent
PROJECT_DIR = SCRIPT_DIR.parent

HUFFMAN    = PROJECT_DIR / "build" / "src" / "huffman"
INPUT_DIR  = SCRIPT_DIR / "data"
OUTPUT_DIR = SCRIPT_DIR / "output"
RESULTS_CSV = SCRIPT_DIR / "results.csv"

TMP_COMPRESSED = OUTPUT_DIR / "tmp.bin"
TMP_RESTORED   = OUTPUT_DIR / "tmp.restored"


"""Настройки"""
FLAG_C = "-c"
FLAG_D = "-d"
N_RUNS = 10
WARMUP = 1

SIZES = ["1kb", "10kb", "100kb", "1mb", "5mb"]
TYPES = ["repeat", "text", "random", "archive", "photo"]


"""Утилиты"""
def run(cmd: list[str]) -> None:
    subprocess.run(cmd, check=True,
                   stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)


def files_are_equal(first: Path, second: Path) -> bool:
    """Побайтовое сравнение двух файлов."""
    return first.read_bytes() == second.read_bytes()


def measure_one(path: Path) -> dict:
    """Прогоняет сжатие/распаковку N_RUNS раз, возвращает статистику"""
    orig_size = os.path.getsize(path)

    for _ in range(WARMUP):
        run([str(HUFFMAN), FLAG_C, str(path), str(TMP_COMPRESSED)])
        run([str(HUFFMAN), FLAG_D, str(TMP_COMPRESSED), str(TMP_RESTORED)])

    compress_times = []
    decompress_times = []

    for _ in range(N_RUNS):
        t0 = time.perf_counter()
        run([str(HUFFMAN), FLAG_C, str(path), str(TMP_COMPRESSED)])
        t1 = time.perf_counter()
        run([str(HUFFMAN), FLAG_D, str(TMP_COMPRESSED), str(TMP_RESTORED)])
        t2 = time.perf_counter()

        compress_times.append(t1 - t0)
        decompress_times.append(t2 - t1)

    compressed_size = os.path.getsize(TMP_COMPRESSED)
    roundtrip_ok = files_are_equal(path, TMP_RESTORED)

    return {
        "file": path.name,
        "orig_size": orig_size,
        "compressed_size": compressed_size,
        "ratio_percent": 100.0 * (1 - compressed_size / orig_size) if orig_size else 0.0,
        "compress_mean_ms":    1000 * statistics.mean(compress_times),
        "compress_stdev_ms":   1000 * statistics.stdev(compress_times) if len(compress_times) > 1 else 0.0,
        "decompress_mean_ms":  1000 * statistics.mean(decompress_times),
        "decompress_stdev_ms": 1000 * statistics.stdev(decompress_times) if len(decompress_times) > 1 else 0.0,
        "roundtrip_ok": roundtrip_ok,
    }


"""Сбор данных"""
def collect() -> list[dict]:
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    files = sorted(INPUT_DIR.glob("*"))
    files = [f for f in files if f.is_file()]

    if not files:
        print(f"Нет файлов в {INPUT_DIR}")
        print(f"Сгенерируйте данные: {SCRIPT_DIR / 'generator.py'}")
        return []

    results = []
    for path in files:
        row = measure_one(path)
        results.append(row)

        status = "OK" if row["roundtrip_ok"] else "FAIL"
        print(f"{row['file']:30s} "
              f"{row['orig_size']:>9} → {row['compressed_size']:>9}  "
              f"({row['ratio_percent']:>7.2f}%)  "
              f"c: {row['compress_mean_ms']:>8.2f}±{row['compress_stdev_ms']:>6.2f} мс  "
              f"d: {row['decompress_mean_ms']:>8.2f}±{row['decompress_stdev_ms']:>6.2f} мс  "
              f"{status}")

    for tmp in (TMP_COMPRESSED, TMP_RESTORED):
        tmp.unlink(missing_ok=True)

    return results


"""Таблицы"""
def print_table(results: list[dict], key: str, title: str, fmt: str) -> None:
    """Печатает таблицу: строки — типы, столбцы — размеры"""
    lookup = {}
    for r in results:
        stem = Path(r["file"]).stem
        if "_" not in stem:
            continue
        data_type, size_label = stem.rsplit("_", 1)
        lookup[(data_type, size_label)] = r

    print()
    print(title)
    header = f"{'Тип':<10}" + "".join(f"{s:>12}" for s in SIZES)
    print(header)
    print("-" * len(header))

    for data_type in TYPES:
        row = f"{data_type:<10}"
        for size_label in SIZES:
            record = lookup.get((data_type, size_label))
            if record is None:
                row += f"{'---':>12}"
            else:
                row += f"{fmt.format(record[key]):>12}"
        print(row)


def print_all_tables(results: list[dict]) -> None:
    print_table(results, "ratio_percent",
                "Коэффициент сжатия, % (больше — лучше)", "{:.2f}")
    print_table(results, "compress_mean_ms",
                "Время сжатия, мс (μ)", "{:.2f}")
    print_table(results, "compress_stdev_ms",
                "Время сжатия, мс (σ)", "{:.2f}")
    print_table(results, "decompress_mean_ms",
                "Время распаковки, мс (μ)", "{:.2f}")
    print_table(results, "decompress_stdev_ms",
                "Время распаковки, мс (σ)", "{:.2f}")


"""CSV"""
def save_csv(results: list[dict], path: Path) -> None:
    if not results:
        return
    with open(path, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=results[0].keys())
        writer.writeheader()
        writer.writerows(results)


"""main"""
def main() -> int:
    if not HUFFMAN.exists():
        print(f"Не найден архиватор: {HUFFMAN}")
        print("Соберите проект: cmake --build build -j")
        return 1

    if not INPUT_DIR.exists():
        print(f"Не найдена директория с данными: {INPUT_DIR}")
        print(f"Сгенерируйте данные: {SCRIPT_DIR / 'generator.py'}")
        return 1

    print(f"Архиватор: {HUFFMAN}")
    print(f"Данные:    {INPUT_DIR}")
    print(f"Прогонов:  {N_RUNS} (+{WARMUP} прогрев)\n")
    print("Измерения:")

    results = collect()
    if not results:
        return 1

    failed = [r for r in results if not r["roundtrip_ok"]]
    if failed:
        print(f"\nВНИМАНИЕ: {len(failed)} файлов не восстановлены корректно:")
        for r in failed:
            print(f"  {r['file']}")
        return 1

    print_all_tables(results)

    save_csv(results, RESULTS_CSV)
    print(f"\nДанные сохранены: {RESULTS_CSV}")
    print(f"Для построения графиков: python3 {SCRIPT_DIR / 'graphs.py'}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())