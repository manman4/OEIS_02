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
- $`N_q(n)`$：長さ $`n`$ の cyclically smooth necklace の個数。
- $`C_q(n)`$：長さ $`n`$ の cyclically smooth Lyndon word の個数。

ここで necklace は各回転同値類の辞書式最小代表によって表す。

## 主公式

$`n\ge 1`$ に対して、

```math
S_q(n)=L_q(n)+\sum_{d\mid n,\ d\ne n}C_q(d).
```

後述する $`N_q(n)`$ を使えば、同じ公式をより簡潔に

```math
S_q(n)=L_q(n)+N_q(n)-C_q(n).
```

と書ける。

また、空語に対応する初期値は

```math
S_q(0)=1
```

である。

## cyclically smooth necklaces と Möbius 反転

長さ $`n`$ の cyclically smooth necklace は、ある $`d\mid n`$ に対する長さ $`d`$ の cyclically smooth Lyndon word の反復として一意に表される。したがって、

```math
N_q(n)=\sum_{d\mid n}C_q(d).
```

Möbius 反転により、$`C_q(n)`$ は次の簡単な形になる。

```math
C_q(n)=\sum_{d\mid n}\mu(n/d)N_q(d).
```

$`q=3`$ では $`N_3(n)`$ が [A208772](https://oeis.org/A208772)、$`C_3(n)`$ が [A215335](https://oeis.org/A215335) であり、これは A215335 に掲載されている公式そのものである。主公式はさらに、$`N_q`$ だけを使って

```math
S_q(n)=L_q(n)-\sum_{d\mid n,\ d\ne n}\mu(n/d)N_q(d).
```

とも書ける。

## cyclically smooth necklaces の閉形式

$`q\times q`$ 行列 $`M_q`$ を次のように定義する。$`0\le r,s\le q-1`$ に対し、$`|r-s|\le1`$ なら $`(M_q)_{rs}=1`$、それ以外なら $`(M_q)_{rs}=0`$ とする。この行列の固有値は

```math
\lambda_j=1+2\cos(\frac{j\pi}{q+1}), (1\le j\le q).
```

である。Burnside の補題により、

```math
N_q(n)=\frac{1}{n}\sum_{e\mid n}\varphi(e)\sum_{j=1}^{q}(1+2\cos(\frac{j\pi}{q+1}))^{n/e}.
```

$`N_q`$ を介さず $`C_q`$ を直接計算する場合は、これと同値な次の式を使える。

```math
C_q(n)=\frac{1}{n}\sum_{e\mid n}\mu(e)\sum_{j=1}^{q}(1+2\cos(\frac{j\pi}{q+1}))^{n/e}.
```

したがって $`S_q`$、$`L_q`$、$`N_q`$、$`C_q`$ の関係は、個別の OEIS 数列を使わず $`S_q=L_q+N_q-C_q`$ と上の2公式で記述できる。

## 証明

### 原始部分

長さ $`n`$ の necklace を、最小周期が $`n`$ である原始的なものと、最小周期が $`n`$ より小さい周期的なものに分ける。

原始 necklace の辞書式最小代表は Lyndon word である。したがって、smooth な原始 necklace の個数は $`L_q(n)`$ である。

### 周期部分についての補題

周期的な necklace の辞書式最小代表を $`w`$、その最小周期を $`d`$ とする。このとき、長さ $`d`$ の Lyndon word $`u=u_1u_2\cdots u_d`$ と $`k=n/d\ge2`$ を用いて

```math
w=u^k
```

と一意に表される。

$`w`$ の内部に現れる隣接対には、各コピーの内部の

```math
(u_1,u_2),(u_2,u_3),\dots,(u_{d-1},u_d)
```

だけでなく、あるコピーの末尾と次のコピーの先頭から生じる $`(u_d,u_1)`$ も含まれる。$`k\ge2`$ なので、この境界の隣接対は必ず $`w`$ の内部に少なくとも1回現れる。

したがって、$`w`$ が smooth であるための必要十分条件は、$`u`$ が cyclically smooth であることである。また、$`w`$ の末尾と先頭の対も同じ $`(u_d,u_1)`$ なので、周期的な necklace に限れば、smooth であることと cyclically smooth であることは同値である。

### 個数の対応

$`N_q(n)`$ はすべての cyclically smooth necklace を数え、そのうち原始的なものはちょうど cyclically smooth Lyndon words なので、その個数は $`C_q(n)`$ である。よって周期的な cyclically smooth necklace の個数は

```math
N_q(n)-C_q(n).
```

上の補題により、これは周期的な smooth necklace の個数でもある。したがって、smooth necklace を原始部分と周期部分に分けると、

```math
S_q(n)=L_q(n)+(N_q(n)-C_q(n)).
```

すなわち、

```math
S_q(n)=L_q(n)+N_q(n)-C_q(n).
```

を得る。

また、cyclically smooth necklace を最小周期 $`d\mid n`$ で分類すると $`N_q(n)=\sum_{d\mid n}C_q(d)`$ となる。Möbius 反転によって $`C_q(n)=\sum_{d\mid n}\mu(n/d)N_q(d)`$ を得る。さらに $`N_q(n)-C_q(n)=\sum_{d\mid n,\ d\ne n}C_q(d)`$ なので、この簡潔な主公式は最初に示した約数和の公式とも等価である。

さらに、行列 $`M_q^m`$ のトレースは長さ $`m`$ の cyclically smooth な閉路付き語の個数であり、その値は固有値の $`m`$ 乗和である。回転による固定点を Burnside の補題で平均すると上記の $`N_q(n)`$ の閉形式を得る。周期が $`n`$ である閉路付き語を Möbius 反転で取り出し、1つの原始回転同値類に含まれる $`n`$ 個の回転を除けば、$`C_q(n)`$ の直接公式も得られる。

## 素数長の場合

$`p`$ が素数なら真の正の約数は $`1`$ だけであり、$`C_q(1)=q`$ なので、

```math
S_q(p)=L_q(p)+q.
```

## OEIS 対応表

$`S_q`$、$`L_q`$、$`N_q`$、$`C_q`$ に対応する、確認できた OEIS の個別数列は次の通りである。

| 色数 $`q`$ | $`S_q(n)`$: smooth necklaces | $`L_q(n)`$: smooth Lyndon words | $`N_q(n)`$: cyclically smooth necklaces | $`C_q(n)`$: cyclically smooth Lyndon words |
|---:|:---|:---|:---|:---|
| 2 | [A000031](https://oeis.org/A000031) | [A001037](https://oeis.org/A001037) | [A000031](https://oeis.org/A000031) | [A001037](https://oeis.org/A001037) |
| 3 | [A215327](https://oeis.org/A215327) | [A215328](https://oeis.org/A215328) | [A208772](https://oeis.org/A208772) | [A215335](https://oeis.org/A215335) |
| 4 | [A215329](https://oeis.org/A215329) | [A215330](https://oeis.org/A215330) | [A208773](https://oeis.org/A208773) | [A215336](https://oeis.org/A215336) |
| 5 | [A215331](https://oeis.org/A215331) | [A215332](https://oeis.org/A215332) | [A208774](https://oeis.org/A208774) | [A215337](https://oeis.org/A215337) |
| 6 | 専用項目を確認できない | 専用項目を確認できない | [A208775](https://oeis.org/A208775) | 専用項目を確認できない |
| 7 | [A215333](https://oeis.org/A215333) | [A215334](https://oeis.org/A215334) | [A208776](https://oeis.org/A208776) | [A215338](https://oeis.org/A215338) |

$`q=2`$ では任意の隣接2文字の差が高々1なので、すべての binary necklace および binary Lyndon word が自動的に smooth である。このため $`S_2=N_2`$ および $`L_2=C_2`$ となる。

$`q=1`$ では $`S_1(n)=N_1(n)=1`$ であり、$`n\ge1`$ に対して $`L_1(n)=C_1(n)=1`$ は $`n=1`$ のときだけ、それ以外では0である。この場合の $`S_1`$ と $`N_1`$ は [A000012](https://oeis.org/A000012) に一致する。

OEIS で $`S_q`$、$`L_q`$、$`C_q`$ の3系列すべてに専用項目を確認できる非自明な色数は $`q=3,4,5,7`$ である。$`N_q(n)`$ は一般に [A208777](https://oeis.org/A208777) の第 $`q`$ 列であり、$`q=6`$ にも専用項目 A208775 がある。$`q=6`$ および $`q\ge8`$ についても一般公式は成立するが、対応する専用の $`S_q`$、$`L_q`$、$`C_q`$ の項目は確認できない。

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

$`q=3`$ とすると $`S_3(n)`$ は [A215327](https://oeis.org/A215327)、$`L_3(n)`$ は [A215328](https://oeis.org/A215328)、$`N_3(n)`$ は [A208772](https://oeis.org/A208772)、$`C_3(n)`$ は [A215335](https://oeis.org/A215335) に対応する。

例えば $`n=6`$ では、

```math
S_3(6)=L_3(6)+N_3(6)-C_3(6)=49+39-30=58.
```

## 4色の場合

$`q=4`$ とすると $`S_4(n)`$ は [A215329](https://oeis.org/A215329)、$`L_4(n)`$ は [A215330](https://oeis.org/A215330)、$`N_4(n)`$ は [A208773](https://oeis.org/A208773)、$`C_4(n)`$ は [A215336](https://oeis.org/A215336) に対応する。

したがって、参考数列 A215329 にも同じ形の公式が成り立つ。

## 注意

cyclically smooth という条件は回転不変なので、隣接行列と Burnside の補題または Möbius 反転による閉形式が得られる。一方、A215327 型の smooth 条件は末尾と先頭を比較せず、辞書式最小代表に依存するため回転不変ではない。そのため、一般の $`L_q(n)`$ を通常の necklace 多項式だけで置き換えることはできない。

## 参考文献

- Arnold Knopfmacher, Toufik Mansour, Augustine Munagi, and Helmut Prodinger, [Smooth words and Chebyshev polynomials](https://arxiv.org/abs/0809.0551), 2008.
- [A208772](https://oeis.org/A208772), cyclically smooth necklaces with 3 colors.
