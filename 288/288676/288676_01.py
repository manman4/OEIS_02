#!/usr/bin/env python3
"""b108394.txt から A288676 の b-file を作る。

A288676 は、A108394(k) が素数でないような k を小さい順に並べた数列。
"""

from __future__ import annotations

import argparse
import math
from pathlib import Path
from typing import Iterator


def is_prime(n: int) -> bool:
    """n が素数なら True を返す。"""
    if n < 2:
        return False
    if n % 2 == 0:
        return n == 2

    limit = math.isqrt(n)
    for divisor in range(3, limit + 1, 2):
        if n % divisor == 0:
            return False
    return True


def read_bfile(path: Path) -> Iterator[tuple[int, int]]:
    """b-file の (添字, 値) を順に読み取る。"""
    previous_index: int | None = None

    with path.open(encoding="utf-8-sig") as source:
        for line_number, line in enumerate(source, start=1):
            body = line.partition("#")[0].strip()
            if not body:
                continue

            fields = body.split()
            if len(fields) != 2:
                raise ValueError(
                    f"{path}:{line_number}: 2 列の整数ではありません: {line.rstrip()}"
                )
            try:
                index, value = map(int, fields)
            except ValueError as error:
                raise ValueError(
                    f"{path}:{line_number}: 整数として解釈できません: {line.rstrip()}"
                ) from error

            if previous_index is not None and index <= previous_index:
                raise ValueError(
                    f"{path}:{line_number}: 添字 {index} が昇順になっていません"
                )
            previous_index = index
            yield index, value


def make_a288676(input_path: Path, output_path: Path) -> tuple[int, int, int]:
    """A288676 を計算して出力し、(入力数, 出力数, 最大添字) を返す。"""
    checked = 0
    found = 0
    max_index = 0

    with output_path.open("w", encoding="utf-8", newline="\n") as output:
        output.write(f"# {output_path.name}\n")
        output.write("# A288676: Numbers k such that A108394(k) is not a prime.\n")
        output.write(f"# Computed from {input_path.name}\n")

        for index, value in read_bfile(input_path):
            checked += 1
            max_index = index
            if not is_prime(value):
                found += 1
                output.write(f"{found} {index}\n")

        output.write(f"# a({found + 1}) > {max_index}\n")

    return checked, found, max_index


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="b108394.txt から A288676 の b-file を作成します。"
    )
    parser.add_argument(
        "input",
        nargs="?",
        type=Path,
        default=Path("b108394.txt"),
        help="入力 b-file（既定: b108394.txt）",
    )
    parser.add_argument(
        "-o",
        "--output",
        type=Path,
        default=Path("b288676_01.txt"),
        help="出力 b-file（既定: b288676_01.txt）",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    checked, found, max_index = make_a288676(args.input, args.output)
    print(f"{checked} 項を判定しました（入力の最大添字: {max_index}）")
    print(f"A288676 の {found} 項を {args.output} に出力しました")
    print(f"次の項 a({found + 1}) > {max_index}")


if __name__ == "__main__":
    main()
