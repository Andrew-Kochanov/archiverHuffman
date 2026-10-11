import os
import zipfile
from pathlib import Path


SIZES = {
    "1kb":    1 * 1024,
    "10kb":  10 * 1024,
    "100kb": 100 * 1024,
    "1mb":   1 * 1024 * 1024,
    "5mb":   5 * 1024 * 1024,
}

OUT_DIR = Path(__file__).resolve().parent / "data"

TEXT_CORPUS = (
    "Call me Ishmael. Some years ago—never mind how long precisely—having "
    "little or no money in my purse, and nothing particular to interest me "
    "on shore, I thought I would sail about a little and see the watery part "
    "of the world. It is a way I have of driving off the spleen and "
    "regulating the circulation. Whenever I find myself growing grim about "
    "the mouth; whenever it is a damp, drizzly November in my soul; "
    "whenever I find myself involuntarily pausing before coffin warehouses, "
    "and bringing up the rear of every funeral I meet; and especially "
    "whenever my hypos get such an upper hand of me, that it requires a "
    "strong moral principle to prevent me from deliberately stepping into "
    "the street, and methodically knocking people's hats off—then, I account "
    "it high time to get to sea as soon as I can. This is my substitute for "
    "pistol and ball. With a philosophical flourish Cato throws himself upon "
    "his sword; I quietly take to the ship. There is nothing surprising in "
    "this. If they but knew it, almost all men in their degree, some time "
    "or other, cherish very nearly the same feelings towards the ocean with "
    "me."
)

def make_repeat(size,name):
    """Повторяющийся текст"""
    path = OUT_DIR / f"repeat_{name}.txt"
    path.write_bytes(b"r" * size)
    return path

def make_text(size,name):
    """Обычный текст с естественным распределением символов"""
    corpus = TEXT_CORPUS.encode("utf-8")
    data = (corpus * (size // len(corpus) + 1))[:size]
    path = OUT_DIR / f"text_{name}.txt"
    path.write_bytes(data)
    return path


def make_random(size,name):
    """Случайные байты — все 256 символов равновероятны"""
    path = OUT_DIR / f"random_{name}.bin"
    path.write_bytes(os.urandom(size))
    return path


def make_archive(size, name):
    """ZIP-архив"""
    path = OUT_DIR / f"archive_{name}.zip"

    corpus_bytes = TEXT_CORPUS.encode("utf-8")
    corpus_len = len(corpus_bytes)
    payload_bytes = size * 4

    for _ in range(15):
        with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
            with z.open("document.txt", "w") as f:
                remaining = payload_bytes
                while remaining > 0:
                    chunk = corpus_bytes[:min(corpus_len, remaining)]
                    f.write(chunk)
                    remaining -= len(chunk)

        actual = path.stat().st_size
        if abs(actual - size) < max(64, size * 0.02):
            break

        payload_bytes = max(1, int(payload_bytes * size / actual))

    return path

def main():
    OUT_DIR.mkdir(parents=True, exist_ok=True)

    generators = [
        make_repeat,
        make_text,
        make_random,
        make_archive,
    ]

    for generator in generators:
        print(f"[{generator.__name__}]")
        for name in SIZES:
            size = SIZES[name]
            path = generator(size, name)
            print(f"  {path.name:24s} {path.stat().st_size:>10d} байт")
        print()

    print("Ready")


if __name__ == "__main__":
    main()