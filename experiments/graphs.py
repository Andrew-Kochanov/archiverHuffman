import csv
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


"""Пути"""
SCRIPT_DIR  = Path(__file__).resolve().parent
RESULTS_CSV = SCRIPT_DIR / "results.csv"
GRAPH_PNG   = SCRIPT_DIR / "graphs.png"


"""Настройки"""
SIZES = ["1kb", "10kb", "100kb", "1mb", "5mb"]
TYPES = ["repeat", "text", "random", "archive", "photo"]

SIZE_BYTES = {
    "1kb":    1 * 1024,
    "10kb":  10 * 1024,
    "100kb": 100 * 1024,
    "1mb":   1 * 1024 * 1024,
    "5mb":   5 * 1024 * 1024,
}

STYLES = {
    "repeat":  {"color": "#1f77b4", "linestyle": "-",  "marker": "o"},
    "text":    {"color": "#ff7f0e", "linestyle": "--", "marker": "s"},
    "random":  {"color": "#2ca02c", "linestyle": ":",  "marker": "^"},
    "archive": {"color": "#d62728", "linestyle": "-.", "marker": "v"},
    "photo":   {"color": "#9467bd", "linestyle": "-",  "marker": "D"},
}


"""Загрузка данных"""
def load_results(path: Path) -> list[dict]:
    """Читает CSV, возвращает список словарей с приведёнными типами."""
    if not path.exists():
        raise FileNotFoundError(
            f"Не найден {path}. Сначала запустите experiments.py."
        )

    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append({
                "file": row["file"],
                "orig_size": int(row["orig_size"]),
                "compressed_size": int(row["compressed_size"]),
                "ratio_percent": float(row["ratio_percent"]),
                "compress_mean_ms": float(row["compress_mean_ms"]),
                "compress_stdev_ms": float(row["compress_stdev_ms"]),
                "decompress_mean_ms": float(row["decompress_mean_ms"]),
                "decompress_stdev_ms": float(row["decompress_stdev_ms"]),
                "roundtrip_ok": row["roundtrip_ok"].strip().lower() in ("true", "1"),
            })
    return rows


def index_by_type_and_size(results: list[dict]) -> dict:
    lookup = {}
    for r in results:
        stem = Path(r["file"]).stem
        if "_" not in stem:
            continue
        data_type, size_label = stem.rsplit("_", 1)
        lookup[(data_type, size_label)] = r
    return lookup


"""Графики"""
def build_graphs(results: list[dict], out_path: Path) -> None:
    lookup = index_by_type_and_size(results)

    fig, axes = plt.subplots(1, 3, figsize=(20, 6))

    for data_type in TYPES:
        style = STYLES[data_type]
        xs, ratios, comp, decomp = [], [], [], []

        for size_label in SIZES:
            record = lookup.get((data_type, size_label))
            if record is None:
                continue
            xs.append(SIZE_BYTES[size_label])
            ratios.append(record["ratio_percent"])
            comp.append(record["compress_mean_ms"])
            decomp.append(record["decompress_mean_ms"])

        if not xs:
            continue

        axes[0].plot(xs, ratios, label=data_type,
                     linewidth=2, markersize=7, **style)
        axes[1].plot(xs, comp, label=data_type,
                     linewidth=2, markersize=7, **style)
        axes[2].plot(xs, decomp, label=data_type,
                     linewidth=2, markersize=7, **style)

    """График коэффициента сжатия"""
    axes[0].set_xscale("log")
    axes[0].set_xlabel("Размер файла, байт")
    axes[0].set_ylabel("Экономия, % (1 − сжатый/исходный)")
    axes[0].set_title("Степень сжатия")
    axes[0].axhline(y=0, color="gray", linestyle="--",
                    linewidth=1, alpha=0.7, zorder=0)
    axes[0].grid(True, which="both", alpha=0.3)
    axes[0].legend(fontsize=10, loc="center left")

    """Графики времени"""
    for ax, title, ylabel in [
        (axes[1], "Время сжатия", "мс"),
        (axes[2], "Время распаковки", "мс"),
    ]:
        ax.set_xscale("log")
        ax.set_yscale("log")
        ax.set_xlabel("Размер файла, байт")
        ax.set_ylabel(ylabel)
        ax.set_title(title)
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=10)

    plt.tight_layout()
    plt.savefig(out_path, dpi=150)
    plt.close()
    print(f"График сохранён: {out_path}")

def main() -> int:
    try:
        results = load_results(RESULTS_CSV)
    except FileNotFoundError as error:
        print(error)
        return 1

    if not results:
        print(f"В {RESULTS_CSV} нет данных.")
        return 1

    build_graphs(results, GRAPH_PNG)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())