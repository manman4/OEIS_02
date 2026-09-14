# Smooth necklaces の一般公式

## 定義

$`q\ge 1`$ とし、順序付きアルファベットを

```math
A_q=\{0,1,\dots,q-1\}.
```

とする。

長さ $`n`$ の語 $`w=w_1w_2\cdots w_n`$ が **smooth** であるとは、

```math
|w_i-w_{i-1}|\le 1, (2\le i\le n).
```

を満たすことをいう。さらに

```math
|w_n-w_1|\le 1
```

も満たすとき、$`w`$ は **cyclically smooth** であるという。

次の個数を定義する。

- $`S_q(n)`$：長さ $`n`$ の smooth necklace の個数。
- $`L_q(n)`$：長さ $`n`$ の smooth Lyndon word の個数。
- $`C_q(n)`$：長さ $`n`$ の cyclically smooth Lyndon word の個数。

ここで necklace は各回転同値類の辞書式最小代表によって表す。

## 主公式

$`n\ge 1`$ に対して、

```math
S_q(n)=L_q(n)+\sum_{d\mid n,\ d\ne n}C_q(d).
```

また、空語に対応する初期値は

```math
S_q(0)=1
```

である。

## 巡回的に smooth な Lyndon word の公式

$`q\times q`$ 行列 $`M_q`$ を次のように定義する。$`0\le r,s\le q-1`$ に対し、$`|r-s|\le1`$ なら $`(M_q)_{rs}=1`$、それ以外なら $`(M_q)_{rs}=0`$ とする。この行列の固有値は

```math
\lambda_j=1+2\cos(\frac{j\pi}{q+1}), (1\le j\le q).
```

である。したがって Möbius 反転により、

```math
C_q(n)=\frac{1}{n}\sum_{e\mid n}\mu(e)\sum_{j=1}^{q}(1+2\cos(\frac{j\pi}{q+1}))^{n/e}.
```

これを主公式へ代入すると、個別の OEIS 数列を参照しない一般形

```math
S_q(n)=L_q(n)+\sum_{d\mid n,\ d\ne n}\frac{1}{d}\sum_{e\mid d}\mu(e)\sum_{j=1}^{q}(1+2\cos(\frac{j\pi}{q+1}))^{d/e}.
```

を得る。

## 証明

長さ $`n`$ の necklace の最小周期を $`d`$ とする。辞書式最小代表は、長さ $`d`$ の Lyndon word $`u`$ を用いて

```math
w=u^{n/d}
```

と一意に表される。

### 非周期の場合

$`d=n`$ なら $`w=u`$ である。この場合に数えるべきものは、長さ $`n`$ の smooth Lyndon words であり、その個数は $`L_q(n)`$ である。

### 周期的な場合

$`d`$ が $`n`$ の真の約数なら $`u`$ は少なくとも2回反復される。各コピーの内部だけでなく、あるコピーの末尾 $`u_d`$ と次のコピーの先頭 $`u_1`$ も $`w`$ の隣接文字になる。したがって $`w`$ が smooth であるための必要十分条件は、$`u`$ が cyclically smooth であることである。

よって各真の約数 $`d\mid n`$ から $`C_q(d)`$ 個が寄与し、主公式が従う。

さらに、行列 $`M_q^m`$ のトレースは長さ $`m`$ の cyclically smooth な閉路付き語の個数であり、その値は固有値の $`m`$ 乗和である。周期が $`n`$ である閉路付き語を Möbius 反転で取り出し、1つの原始回転同値類に含まれる $`n`$ 個の回転を除くため $`n`$ で割ると、上記の $`C_q(n)`$ の公式を得る。

## 素数長の場合

$`p`$ が素数なら真の正の約数は $`1`$ だけであり、$`C_q(1)=q`$ なので、

```math
S_q(p)=L_q(p)+q.
```

## OEIS 対応表

$`S_q`$、$`L_q`$、$`C_q`$ に対応する、確認できた OEIS の個別数列は次の通りである。

| 色数 $`q`$ | $`S_q(n)`$: smooth necklaces | $`L_q(n)`$: smooth Lyndon words | $`C_q(n)`$: cyclically smooth Lyndon words |
|---:|:---|:---|:---|
| 2 | [A000031](https://oeis.org/A000031) | [A001037](https://oeis.org/A001037) | [A001037](https://oeis.org/A001037) |
| 3 | [A215327](https://oeis.org/A215327) | [A215328](https://oeis.org/A215328) | [A215335](https://oeis.org/A215335) |
| 4 | [A215329](https://oeis.org/A215329) | [A215330](https://oeis.org/A215330) | [A215336](https://oeis.org/A215336) |
| 5 | [A215331](https://oeis.org/A215331) | [A215332](https://oeis.org/A215332) | [A215337](https://oeis.org/A215337) |
| 6 | 専用項目を確認できない | 専用項目を確認できない | 専用項目を確認できない |
| 7 | [A215333](https://oeis.org/A215333) | [A215334](https://oeis.org/A215334) | [A215338](https://oeis.org/A215338) |

$`q=2`$ では任意の隣接2文字の差が高々1なので、すべての binary necklace および binary Lyndon word が自動的に smooth である。このため $`L_2=C_2`$ となる。

$`q=1`$ では $`S_1(n)=1`$ であり、$`n\ge1`$ に対して $`L_1(n)=C_1(n)=1`$ は $`n=1`$ のときだけ、それ以外では0である。この場合の $`S_1`$ は [A000012](https://oeis.org/A000012) に一致する。

OEIS でこの名称の系列として確認できる非自明な色数は $`q=3,4,5,7`$ である。$`q=6`$ および $`q\ge8`$ についても、本稿の一般公式によって $`C_q(n)`$ と $`S_q(n)`$ の周期的部分を計算できるが、対応する専用の $`S_q`$、$`L_q`$、$`C_q`$ の項目は確認できない。

## 計算プログラム

プログラムは多倍長整数ライブラリ GMP を使用する。このディレクトリをカレントディレクトリとして、次のようにコンパイルする。

```text
cc -std=c11 -O2 -Wall -Wextra -Wpedantic 215327_01.c $(pkg-config --cflags --libs gmp) -o 215327_01
```

実行形式は次の通りである。`MAX_N` を省略した場合は12とする。

```text
./215327_01 [OPTIONS] Q [MAX_N]
```

通常は $`S_q(n)`$ を b-file と同じ `n value` の2列形式で出力する。例えば3色について長さ10まで計算する場合は、次のようにする。

```text
./215327_01 3 10
```

$`L_q(n)`$ または $`C_q(n)`$ を出力する場合は、次のように指定する。

```text
./215327_01 --sequence L 3 10
./215327_01 --sequence C 3 10
```

$`S_q`$ と $`L_q`$ の通常出力には OEIS の規約に合わせて先頭の `0 1` を含める。$`C_q`$ は $`n=1`$ から出力する。各行は、その $`n`$ の列挙と検算が完了した直後に逐次出力する。

全数列と検査結果を表示するには `--check` を指定する。

```text
./215327_01 --check 3 10
```

プログラムは主公式を各 $`n`$ で検査する。また、$`C_q(n)`$ を隣接行列と Möbius 反転によって独立に再計算し、列挙結果と照合する。計算は厳密な全列挙なので、$`q`$ または $`MAX_N`$ が大きい場合は実行時間が指数的に増加する。

## 3色の場合

$`q=3`$ とすると $`S_3(n)`$ は [A215327](https://oeis.org/A215327)、$`L_3(n)`$ は [A215328](https://oeis.org/A215328)、$`C_3(n)`$ は [A215335](https://oeis.org/A215335) に対応する。

例えば $`n=6`$ では、

```math
S_3(6)=L_3(6)+C_3(1)+C_3(2)+C_3(3)=49+3+2+4=58.
```

## 4色の場合

$`q=4`$ とすると $`S_4(n)`$ は [A215329](https://oeis.org/A215329)、$`L_4(n)`$ は [A215330](https://oeis.org/A215330)、$`C_4(n)`$ は [A215336](https://oeis.org/A215336) に対応する。

したがって、参考数列 A215329 にも同じ形の公式が成り立つ。

## 注意

cyclically smooth という条件は回転不変なので、隣接行列と Burnside の補題または Möbius 反転による閉形式が得られる。一方、A215327 型の smooth 条件は末尾と先頭を比較せず、辞書式最小代表に依存するため回転不変ではない。そのため、一般の $`L_q(n)`$ を通常の necklace 多項式だけで置き換えることはできない。

## 参考文献

- Arnold Knopfmacher, Toufik Mansour, Augustine Munagi, and Helmut Prodinger, [Smooth words and Chebyshev polynomials](https://arxiv.org/abs/0809.0551), 2008.
- [A208772](https://oeis.org/A208772), cyclically smooth necklaces with 3 colors.
