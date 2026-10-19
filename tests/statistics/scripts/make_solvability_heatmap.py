#!/usr/bin/env python3
# /*****************************************************************************
#  * LibreMines                                                                *
#  * Copyright (C) 2020-2026  Bruno Bollos Correa                              *
#  *                                                                           *
#  * This program is free software: you can redistribute it and/or modify      *
#  * it under the terms of the GNU General Public License as published by      *
#  * the Free Software Foundation, either version 3 of the License, or         *
#  * (at your option) any later version.                                       *
#  *                                                                           *
#  * This program is distributed in the hope that it will be useful,           *
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
#  * GNU General Public License for more details.                              *
#  *                                                                           *
#  * You should have received a copy of the GNU General Public License         *
#  * along with this program.  If not, see <http://www.gnu.org/licenses/>.     *
#  *****************************************************************************/

"""Render solvability heatmaps from a solver_solvability_statistics report.

The per-starting-point JSON produced by ``solver_solvability_statistics`` is
turned into one heatmap per difficulty, using a red (0% solvable) to green
(100% solvable) colour scale, so it is easy to spot the starting cells that are
friendlier or harsher to the solver.

Dependencies: ``matplotlib`` and ``numpy`` (``pip install matplotlib numpy``).

Example::

    python3 tests/statistics/scripts/make_solvability_heatmap.py \
        --input build/solver_solvability_by_start.json --output-dir heatmaps
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")  # Headless: never require a display.

import matplotlib.pyplot as plt
import numpy as np
from matplotlib import ticker
from matplotlib.colors import Colormap
from matplotlib.colors import LinearSegmentedColormap
from matplotlib.figure import Figure

#: A pure-red to pure-green gradient, used when the caller asks for it via
#: ``--cmap red-green``. ``RdYlGn`` (the default) keeps mid values readable.
RED_GREEN = LinearSegmentedColormap.from_list("red_green", ["#d73027", "#1a9850"])
CMAP_ALIASES = {"red-green": RED_GREEN, "red_green": RED_GREEN}

#: Default report file, matching the default output name of the C++ tool.
DEFAULT_INPUT = "solver_solvability_by_start.json"


def parse_arguments(argv: list[str]) -> argparse.Namespace:
    """Parse the command line arguments."""
    parser = argparse.ArgumentParser(
        description="Render solvability heatmaps (red = 0%, green = 100%) "
                    "from a solver_solvability_statistics JSON report.",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter,
    )
    parser.add_argument(
        "input_positional",
        nargs="?",
        default=DEFAULT_INPUT,
        metavar="input",
        help="JSON report produced by solver_solvability_statistics.",
    )
    parser.add_argument(
        "-i", "--input",
        dest="input_option",
        default=argparse.SUPPRESS,
        help="JSON report (same as the positional argument).",
    )
    parser.add_argument(
        "-o", "--output-dir",
        default=".",
        help="Directory where the PNG images are written.",
    )
    parser.add_argument(
        "-d", "--difficulties",
        default="",
        help="Comma separated subset of levels (default: every level in the file).",
    )
    parser.add_argument(
        "--cmap",
        default="RdYlGn",
        help="Matplotlib colormap, or the special value 'red-green'.",
    )
    parser.add_argument(
        "--no-annotate",
        action="store_true",
        help="Do not draw the percentage inside each cell.",
    )
    parser.add_argument(
        "--dpi",
        type=int,
        default=150,
        help="Resolution of the generated images.",
    )
    parser.add_argument(
        "--figsize",
        default="",
        help="Override the figure size, given as WIDTHxHEIGHT inches.",
    )
    parser.add_argument(
        "--combined",
        action="store_true",
        help="Also write a single figure with all levels side by side.",
    )
    return parser.parse_args(argv)


def load_report(path: Path) -> dict:
    """Load and validate the JSON report."""
    try:
        with path.open("r", encoding="utf-8") as handle:
            report = json.load(handle)
    except FileNotFoundError:
        sys.exit(f"error: input file not found: {path}")
    except json.JSONDecodeError as error:
        sys.exit(f"error: '{path}' is not valid JSON: {error}")

    if not isinstance(report, dict) or not isinstance(report.get("difficulties"), list):
        sys.exit(f"error: '{path}' does not look like a solvability report")

    return report


def select_levels(report: dict, requested: str) -> list[dict]:
    """Return the list of difficulty dictionaries to render."""
    levels = report["difficulties"]
    if not requested.strip():
        return levels

    wanted = [name.strip().lower() for name in requested.split(",") if name.strip()]
    selected = [level for level in levels if str(level.get("name", "")).lower() in wanted]

    missing = [name for name in wanted
               if name not in {str(level.get("name", "")).lower() for level in levels}]
    if missing:
        available = ", ".join(str(level.get("name", "?")) for level in levels)
        sys.exit(f"error: unknown difficulty '{missing[0]}' (available: {available})")
    if not selected:
        sys.exit("error: no difficulty selected")

    return selected


def level_matrix(level: dict) -> np.ndarray:
    """Turn ``rateMatrix[x][y]`` into a ``(rows, cols)`` array for plotting."""
    cols = int(level["cols"])
    rows = int(level["rows"])
    rate_matrix = level["rateMatrix"]

    if len(rate_matrix) != cols or any(len(column) != rows for column in rate_matrix):
        sys.exit("error: rateMatrix dimensions do not match the level size")

    # rateMatrix is indexed as [x][y]; transpose to [y][x] so that x is the
    # horizontal axis and y the vertical one.
    return np.array(rate_matrix, dtype=float).T


def resolve_cmap(name: str) -> Colormap:
    """Resolve a colormap name, accepting the 'red-green' shortcut."""
    shortcut = CMAP_ALIASES.get(name.lower())
    if shortcut is not None:
        return shortcut
    try:
        return plt.get_cmap(name)
    except ValueError:
        sys.exit(f"error: unknown colormap '{name}'")


def text_colour(cmap: Colormap, value: float) -> str:
    """Pick a readable label colour for a cell filled with ``cmap(value)``."""
    red, green, blue, _ = cmap(float(np.clip(value, 0.0, 1.0)))
    luminance = 0.299 * red + 0.587 * green + 0.114 * blue
    return "black" if luminance > 0.55 else "white"


def auto_figsize(cols: int, rows: int) -> tuple[float, float]:
    """Choose a figure size with roughly half an inch per cell."""
    return cols * 0.5 + 2.0, rows * 0.5 + 2.0


def annotate_fontsize(cols: int, rows: int) -> float:
    """Pick a cell label size that stays legible on large boards."""
    largest = max(cols, rows)
    if largest >= 30:
        return 6.0
    if largest >= 24:
        return 7.0
    if largest >= 16:
        return 8.0
    return 9.0


def style_axes(axes, level: dict, matrix: np.ndarray, cmap: Colormap,
               annotate: bool) -> None:
    """Draw the cell grid, annotations, ticks and labels for one heatmap."""
    cols = int(level["cols"])
    rows = int(level["rows"])

    axes.set_xticks(np.arange(-0.5, cols, 1.0), minor=True)
    axes.set_yticks(np.arange(-0.5, rows, 1.0), minor=True)
    axes.grid(which="minor", color="#404040", linewidth=0.4)
    axes.tick_params(which="minor", length=0)

    # Number every cell for small boards, every other one for wide boards.
    step = 1 if cols <= 20 else 2
    axes.set_xticks(range(0, cols, step))
    axes.set_yticks(range(0, rows, 1 if rows <= 20 else 2))
    axes.tick_params(which="major", length=0, labelsize=8)
    axes.set_xlabel("Column (x)")
    axes.set_ylabel("Row (y)")

    if annotate:
        fontsize = annotate_fontsize(cols, rows)
        for y in range(rows):
            for x in range(cols):
                value = float(matrix[y, x])
                axes.text(x, y, f"{value:.0%}", ha="center", va="center",
                          fontsize=fontsize, color=text_colour(cmap, value))

    axes.set_title(level_title(level))


def level_title(level: dict) -> str:
    """Build the title of a single heatmap."""
    name = str(level.get("name", "?"))
    cols = int(level["cols"])
    rows = int(level["rows"])
    mines = int(level.get("mines", 0))
    rate = float(level.get("solvabilityRate", 0.0))
    boards = int(level.get("boardsTested", 0)) // max(1, int(level.get("cells", 1)))

    subtitle = ""
    if "strategy" in level:
        subtitle = f"strategy: {level['strategy']}\n"
    return (f"LibreMines solvability \u2014 {name} ({cols}\u00d7{rows}, {mines} mines)\n"
            f"{subtitle}overall {rate:.1%} \u00b7 {boards} boards per starting point")


def add_colourbar(figure: Figure, image, axes) -> None:
    """Attach a percentage colourbar to a heatmap."""
    bar = figure.colorbar(image, ax=axes, fraction=0.046, pad=0.04)
    bar.set_label("Solvability")
    bar.ax.yaxis.set_major_formatter(ticker.PercentFormatter(xmax=1.0, decimals=0))
    bar.set_ticks([0.0, 0.25, 0.5, 0.75, 1.0])


def render_single(level: dict, cmap: Colormap, annotate: bool,
                  figsize: tuple[float, float] | None, dpi: int,
                  output_path: Path) -> None:
    """Render and save one heatmap."""
    matrix = level_matrix(level)
    rows, cols = matrix.shape

    figure, axes = plt.subplots(figsize=figsize or auto_figsize(cols, rows))
    image = axes.imshow(matrix, cmap=cmap, vmin=0.0, vmax=1.0,
                        origin="upper", aspect="equal", interpolation="nearest")
    style_axes(axes, level, matrix, cmap, annotate)
    add_colourbar(figure, image, axes)
    figure.savefig(output_path, dpi=dpi, bbox_inches="tight")
    plt.close(figure)


def render_combined(levels: list[dict], cmap: Colormap, annotate: bool,
                    figsize: tuple[float, float] | None, dpi: int,
                    output_path: Path) -> None:
    """Render every selected level as a subplot of a single figure."""
    count = len(levels)
    cols_list = [int(level["cols"]) for level in levels]
    rows_list = [int(level["rows"]) for level in levels]

    if figsize is None:
        # Give each level a width proportional to its column count so every
        # cell keeps a similar, readable size across the figure.
        figsize = (sum(cols_list) * 0.5 + 2.0 * count, max(rows_list) * 0.5 + 3.0)

    figure = plt.figure(figsize=figsize)
    grid = figure.add_gridspec(1, count, width_ratios=cols_list)

    for index, level in enumerate(levels):
        axes = figure.add_subplot(grid[0, index])
        matrix = level_matrix(level)
        image = axes.imshow(matrix, cmap=cmap, vmin=0.0, vmax=1.0,
                            origin="upper", aspect="equal", interpolation="nearest")
        style_axes(axes, level, matrix, cmap, annotate)
        add_colourbar(figure, image, axes)

    figure.suptitle("LibreMines solvability by starting point", fontsize=14)
    figure.savefig(output_path, dpi=dpi, bbox_inches="tight")
    plt.close(figure)


def output_name(level: dict) -> str:
    """Build the default PNG file name for a level."""
    return f"solvability_heatmap_{str(level.get('name', 'level')).lower()}.png"


def parse_figsize(spec: str) -> tuple[float, float] | None:
    """Parse a ``WIDTHxHEIGHT`` figure size, or return ``None`` when empty."""
    if not spec.strip():
        return None
    try:
        width, height = spec.lower().split("x")
        return float(width), float(height)
    except ValueError:
        sys.exit(f"error: --figsize must be WIDTHxHEIGHT, got '{spec}'")


def main(argv: list[str]) -> int:
    """Entry point."""
    args = parse_arguments(argv)
    input_path = Path(getattr(args, "input_option", None)
                      or args.input_positional or DEFAULT_INPUT)
    report = load_report(input_path)
    levels = select_levels(report, args.difficulties)
    cmap = resolve_cmap(args.cmap)
    figsize = parse_figsize(args.figsize)
    annotate = not args.no_annotate

    output_dir = Path(args.output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)

    for level in levels:
        destination = output_dir / output_name(level)
        render_single(level, cmap, annotate, figsize, args.dpi, destination)
        print(f"wrote {destination}")

    if args.combined:
        destination = output_dir / "solvability_heatmap_combined.png"
        render_combined(levels, cmap, annotate, figsize, args.dpi, destination)
        print(f"wrote {destination}")

    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
