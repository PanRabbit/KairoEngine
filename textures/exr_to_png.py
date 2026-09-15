#!/usr/bin/env python3
"""Convert every .exr under this folder (or a given root) to a sibling .png.

Writes linear 8-bit values with no sRGB / gamma. Data maps (normals, roughness,
AO) must stay linear or they wash out when sampled as raw bytes in the engine.

Requires: OpenEXR, numpy, Pillow
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import Imath
import numpy as np
import OpenEXR
from PIL import Image

FLOAT = Imath.PixelType(Imath.PixelType.FLOAT)


def load_channel(exr: OpenEXR.InputFile, name: str, height: int, width: int) -> np.ndarray:
    return np.frombuffer(exr.channel(name, FLOAT), dtype=np.float32).reshape(height, width)


def exr_to_image(path: Path) -> Image.Image:
    exr = OpenEXR.InputFile(str(path))
    header = exr.header()
    dw = header["dataWindow"]
    width = dw.max.x - dw.min.x + 1
    height = dw.max.y - dw.min.y + 1
    channels = header["channels"]

    def ch(name: str) -> np.ndarray:
        return np.clip(load_channel(exr, name, height, width), 0.0, 1.0)

    if all(name in channels for name in ("R", "G", "B")):
        rgb = np.stack([ch("R"), ch("G"), ch("B")], axis=-1)
        if "A" in channels:
            rgba = np.dstack([rgb, ch("A")])
            return Image.fromarray(np.round(rgba * 255.0).astype(np.uint8), mode="RGBA")
        return Image.fromarray(np.round(rgb * 255.0).astype(np.uint8), mode="RGB")

    if "Y" in channels:
        return Image.fromarray(np.round(ch("Y") * 255.0).astype(np.uint8), mode="L")
    if "R" in channels:
        return Image.fromarray(np.round(ch("R") * 255.0).astype(np.uint8), mode="L")

    raise ValueError(f"unsupported EXR channels: {list(channels.keys())}")


def convert_tree(root: Path, force: bool) -> int:
    exrs = sorted(root.rglob("*.exr"))
    if not exrs:
        print(f"no .exr files under {root}")
        return 0

    converted = 0
    for src in exrs:
        dst = src.with_suffix(".png")
        if dst.exists() and not force:
            print(f"skip {src.relative_to(root)} (png exists, use --force)")
            continue
        try:
            image = exr_to_image(src)
            image.save(dst)
            print(f"{src.relative_to(root)} -> {dst.name} ({image.mode} {image.size[0]}x{image.size[1]})")
            converted += 1
        except Exception as exc:
            print(f"FAIL {src.relative_to(root)}: {exc}", file=sys.stderr)
    return converted


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "root",
        nargs="?",
        default=Path(__file__).resolve().parent,
        type=Path,
        help="folder to search recursively (default: this script's directory)",
    )
    parser.add_argument(
        "--force",
        action="store_true",
        help="overwrite existing pngs",
    )
    args = parser.parse_args()
    root = args.root.resolve()
    if not root.is_dir():
        print(f"not a directory: {root}", file=sys.stderr)
        return 1
    convert_tree(root, args.force)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
