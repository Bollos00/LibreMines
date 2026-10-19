# Solver statistics tools

Optional, manually-run tools that measure how well the LibreMines solver
performs. They are **not** part of the normal build, the test suite or CI: they
are only built when you explicitly ask CMake for them.

- **`solver_solvability_statistics`** – for every possible starting cell of the
  Easy/Medium/Hard boards, generates many boards and reports how many are
  solvable. The result is per-starting-point data ready to be drawn as a
  heatmap, plus the overall solvability of each level.
- **`solver_strategy_benchmark`** – runs every solver strategy combination
  against the *same* set of boards and reports the solvability rate and the
  time taken by each combination.

## Compiling

From a build directory:

```bash
cmake -DBUILD_SOLVER_STATISTICS=ON ..   # implies BUILD_TESTS=ON
make -j$(nproc)
```

Or build only the tools:

```bash
make -j$(nproc) solver_solvability_statistics solver_strategy_benchmark
```

The executables are created next to the other tests:

```
build/tests/statistics/solver_solvability_statistics
build/tests/statistics/solver_strategy_benchmark
```

Requires the same dependencies as the test suite (Qt Core/Widgets).

## Running

Run them from anywhere; the default JSON file is written to the **current
working directory** unless `--output` points elsewhere.

```bash
# Per-starting-point solvability (all difficulties, 1000 boards per cell)
./build/tests/statistics/solver_solvability_statistics

# Strategy comparison and timing (all difficulties, 1000 boards each)
./build/tests/statistics/solver_strategy_benchmark
```

Useful smaller runs while experimenting:

```bash
./build/tests/statistics/solver_solvability_statistics \
    --boards 50 --difficulties easy --threads 4 --output /tmp/solv.json

./build/tests/statistics/solver_strategy_benchmark \
    --boards 100 --difficulties easy,medium --output /tmp/strategies.json
```

Progress is printed to **stderr**, the summary table and the final file path to
**stdout**, and the report itself to the file given by `--output`.

Use `--help` to list the options of either tool.

## Parameters

Both tools accept:

| Option | Default | Meaning |
| --- | --- | --- |
| `-b`, `--boards <count>` | `1000` | Boards generated per starting point (tool 1) or per difficulty (tool 2). |
| `-t`, `--threads <count>` | `0` | Worker threads; `0` uses all CPU cores. |
| `-s`, `--seed <seed>` | `20260101` | Master seed. The same seed reproduces the same solvability results. |
| `-o`, `--output <path>` | see below | Destination JSON file. |
| `-d`, `--difficulties <list>` | `easy,medium,hard` | Comma separated subset; also accepts `0`, `1`, `2`. |

Default output files:

- `solver_solvability_statistics` → `solver_solvability_by_start.json`
- `solver_strategy_benchmark` → `solver_strategy_benchmark.json`

`solver_solvability_statistics` additionally accepts:

| Option | Default | Meaning |
| --- | --- | --- |
| `--strategy <id>` | `all_strategies` | Strategy configuration used for every board (see the ids below). |

### Difficulties

| Name | Board | Mines |
| --- | --- | --- |
| `easy` | 8 × 8 | 10 |
| `medium` | 16 × 16 | 40 |
| `hard` | 30 × 16 | 99 |

### Strategy ids (`--strategy`)

| Id | Configuration |
| --- | --- |
| `none` | No strategy (basic rules and zero clearing only) |
| `basic_patterns` | Basic patterns |
| `basic_advanced` | Basic and advanced patterns |
| `pair_rule` | Pair rule only |
| `basic_advanced_pair` | Basic, advanced and pair rule |
| `enumeration_3` | Enumeration (max clues = 3) only |
| `enumeration_8` | Enumeration (max clues = 8) only |
| `enumeration_unlimited` | Enumeration (max clues = unlimited) only |
| `basic_advanced_pair_enum3` | Basic, advanced, pair rule and enumeration (max clues = 3) |
| `basic_advanced_pair_enum8` | Basic, advanced, pair rule and enumeration (max clues = 8) |
| `basic_advanced_pair_enum_unlimited` | Basic, advanced, pair rule and enumeration (max clues = unlimited) |
| `all_strategies` | Everything, including mine counting |

`solver_strategy_benchmark` always tests the twelve combinations above.

## JSON output

Both files start with the run configuration (`seed`, `threads`, counts, level
dimensions) so a report can be traced back to the command that produced it.

`solver_solvability_by_start.json` contains, per difficulty:

- `solvabilityRate` / `solvable` / `boardsTested` – overall result of the level;
- `byStartingPoint` – one entry per cell (`x`, `y`, `boards`, `solvable`, `rate`);
- `rateMatrix` – the same rates as `rateMatrix[x][y]`, ready for a heatmap.

`solver_strategy_benchmark.json` contains the list of `strategies` and a
`results` array. Each entry has `difficulty`, `strategyId`, `solvable`,
`solvabilityRate` and a `timing` object (`totalMs`, `avgMs`, `minMs`, `maxMs`,
`medianMs`, `p95Ms`).

A typical use is to load the JSON in a small script to render the heatmap or a
strategy comparison chart.

## Heatmap script

`scripts/make_solvability_heatmap.py` turns a `solver_solvability_by_start.json`
report into a heatmap per difficulty, using a red (0% solvable) to green (100%
solvable) color scale. Each cell is annotated with its percentage so it is easy
to see which starting points are friendlier to the solver.

It needs Python 3 with `matplotlib` and `numpy`:

```bash
pip install matplotlib numpy
```

Basic usage (writes `solvability_heatmap_easy.png`, `_medium`, `_hard` into the
current directory):

```bash
python3 tests/statistics/scripts/make_solvability_heatmap.py \
    --input build/solver_solvability_by_start.json --output-dir heatmaps
```

Options:

| Option | Default | Meaning |
| --- | --- | --- |
| `input` / `-i`, `--input` | `solver_solvability_by_start.json` | Report to read. |
| `-o`, `--output-dir` | `.` | Directory where the PNGs are written. |
| `-d`, `--difficulties` | every level in the file | Comma separated subset, e.g. `easy,hard`. |
| `--cmap` | `RdYlGn` | Matplotlib colormap, or `red-green` for a pure red-to-green gradient. |
| `--no-annotate` | off | Hide the per-cell percentage labels. |
| `--dpi` | `150` | Resolution of the generated images. |
| `--figsize WxH` | auto | Override the figure size, in inches. |
| `--combined` | off | Also write all levels side by side as `solvability_heatmap_combined.png`. |

Example of a single level, without labels, as one combined figure:

```bash
python3 tests/statistics/scripts/make_solvability_heatmap.py \
    --input build/solver_solvability_by_start.json -o heatmaps \
    --difficulties hard --no-annotate --combined
```

## Notes

- **Reproducibility**: a fixed `--seed` always produces the same solvability
  numbers, independent of the thread scheduling and of which other levels are
  selected. Only the timing figures vary between runs.
- **Runtime**: the defaults are heavy. Tool 1 with all three levels runs
  800,000 solves (Hard alone is 480,000) and the expensive strategies make it
  slow; reduce `--boards` and/or `--difficulties` for quick runs.
- **Timing**: `solver_strategy_benchmark` measures wall-clock time per solve
  while several threads run in parallel, so it includes CPU contention. Medians
  and totals are more meaningful than individual values.
