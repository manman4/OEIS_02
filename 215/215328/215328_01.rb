#!/usr/bin/env ruby
# frozen_string_literal: true
#
# Smooth Lyndon words with k colors
#
#   k = 2 : A001037   k = 3 : A215328   k = 4 : A215330   k = 5 : A215332   k = 7 : A215334
#   (smooth necklaces:  k = 2 : A000031, k = 3 : A215327, k = 4 : A215329,
#                       k = 5 : A215331, k = 7 : A215333)
#   k = 6 は OEIS に見当たらない
#       Lyndon   : 1, 6, 5, 14, 34, 95, 239, 639, 1660, 4381, 11533, 30539, 80941, ...
#       necklaces: 1, 6, 11, 20, 45, 101, 260, 645, 1690, 4397, 11590, 30545, 81077, ...
#
# 定義 (A215328 より):
#   Lyndon word (x[1],...,x[n]) が smooth  <=>  |x[k] - x[k-1]| <= 1   (2 <= k <= n)
#   Lyndon word = 回転同値類のうち非周期的なものを辞書式最小の代表で表したもの。
#   条件は代表を線形に読んだときのもので、巻き戻りの辺 (x[n], x[1]) は入らない。
#   例: k=3, n=4 の "0012" は smooth Lyndon word だが |2-0| = 2 なので巡回的には smooth でない。
#   2 色の Lyndon word はすべて smooth なので k=2 は A001037 に一致する。
#
# ---------------------------------------------------------------------------
# アルゴリズム
#
# (A) 生成法  … SmoothLyndon.counts
#     FKM (Fredricksen-Kessler-Maiorana) / Duval の prenecklace 生成木を smooth 条件で
#     枝刈りする。prenecklace a[1..t] の周期を p とすると
#         Lyndon word <=> p = t ,   necklace <=> p | t
#     なので、深さ NMAX まで 1 回走査するだけで n = 0..NMAX の
#     Lyndon word / necklace / prenecklace 数が同時に得られる (CAT)。
#
# (B) 分解法  … SmoothLyndon.fast
#     巻き戻りの辺まで smooth (= 巡回的に smooth) という条件は回転不変なので Burnside と
#     Moebius 反転で閉じた式になる。T[i][j] = [ |i-j| <= 1 ] (k x k) として
#         C(k,n) = (1/n) * Sum_{d|n} phi(n/d) * tr(T^d)   (巡回 smooth necklace, k=3 で A208772)
#         D(k,n) = (1/n) * Sum_{d|n} mu(n/d)  * tr(T^d)   (巡回 smooth Lyndon word, k=3 で A215335)
#     残る smooth Lyndon word は「不良な辺がちょうど 1 本、しかもそれが巻き戻り位置」で、
#     代表は x[1] = min かつ x[n] >= x[1] + 2 を満たす。その個数を B(k,n) とすると
#         L(n) = D(k,n) + B(k,n) ,   necklaces(n) = C(k,n) + B(k,n)
#     B(k,n) だけを生成すればよいので生成法の約 2 倍速く、D 側は閉じた式なので検算になる。
#     k=3 では  A215328(n) - A215335(n) = A215327(n) - A208772(n) = B(3,n)
#               B(3,n) = 1, 3, 8, 19, 44, 100, 226, 511, 1150, 2600, ...  (n >= 3)
#
#     necklace 側との関係 (u^(n/d) が smooth <=> u が巡回的に smooth, n/d >= 2 のとき):
#         necklaces(n) = L(n) + Sum_{d|n, d<n} D(k,d)
#     k=3 なら A215328(n) = A215327(n) - Sum_{d|n, d<n} A215335(d)。
#
# 目安 (Ruby): 生成法は k=3 で n=20 が約 2 秒、n=22 が約 11 秒。
# 分解法は k=3 で n=22 が約 6 秒、n=24 が約 30 秒。どちらも計算量は a(n) に比例する。
#
# ---------------------------------------------------------------------------
# 使い方
#   ruby 215328_01.rb [K] [NMAX]          # 生成法で n = 0..NMAX の a(n) (既定 K=3, NMAX=18)
#   ruby 215328_01.rb --all [K] [NMAX]    # prenecklace / necklace / Lyndon をまとめて表示
#   ruby 215328_01.rb --fast K N          # 分解法で a(N) のみ
#   ruby 215328_01.rb --table [KMAX] [NMAX]
#   ruby 215328_01.rb --example [K] [N]   # 長さ N の smooth Lyndon word を列挙 (既定 K=3, N=4)
#   ruby 215328_01.rb --known [NMAX]      # OEIS 既知項との照合 + 内部検算 (既定 NMAX=16)
#   ruby 215328_01.rb --brute [KMAX] [NMAX]  # 総当たりとの照合

module SmoothLyndon
  module_function

  # ---------------- (A) 生成法: smooth prenecklace の枚挙 ----------------

  # smooth な prenecklace を辞書式順にすべて渡す。
  # ブロック引数は (a, t, p): a[1..t] が長さ t、周期 p の smooth prenecklace。
  # a は使い回すので、保存したい場合は呼び出し側で dup すること。
  def each_prenecklace(k, nmax)
    return if k < 1 || nmax < 1
    a = Array.new(nmax + 2, 0)
    gen = nil
    gen = lambda { |t, p|
      # a[t] の候補は a[t-p] 以上 (prenecklace 条件) かつ a[t-1] と差 1 以内 (smooth 条件)
      if t == 1
        lo = 0
        hi = k - 1
      else
        prev = a[t - 1]
        lo = a[t - p] > prev - 1 ? a[t - p] : prev - 1
        hi = prev + 1 < k - 1 ? prev + 1 : k - 1
      end
      c = a[t - p]                       # 周期をそのまま延長する枝
      if c >= lo && c <= hi
        a[t] = c
        yield(a, t, p)
        gen.call(t + 1, p) if t < nmax
      end
      c = a[t - p] + 1                   # 周期を破る枝 (新しい周期は t)
      c = lo if lo > c
      while c <= hi
        a[t] = c
        yield(a, t, t)
        gen.call(t + 1, t) if t < nmax
        c += 1
      end
    }
    gen.call(1, 1)
  end

  # n = 0..nmax の個数を一度に数える。
  #   :lyndon         smooth Lyndon word              (k=3 なら A215328)
  #   :necklaces      smooth necklace                 (k=3 なら A215327)
  #   :prenecklaces   smooth prenecklace
  #   :cyclic_lyndon  巻き戻りの辺まで smooth な Lyndon word (k=3 なら A215335, 検算用)
  def counts(k, nmax)
    lyn = Array.new(nmax + 1, 0)
    nec = Array.new(nmax + 1, 0)
    pre = Array.new(nmax + 1, 0)
    cyl = Array.new(nmax + 1, 0)
    lyn[0] = nec[0] = pre[0] = cyl[0] = 1   # 空語
    each_prenecklace(k, nmax) { |a, t, p|
      pre[t] += 1
      next unless (t % p).zero?               # necklace <=> p | t
      nec[t] += 1
      next unless p == t                      # Lyndon   <=> p = t
      lyn[t] += 1
      cyl[t] += 1 if (a[t] - a[1]).abs <= 1
    }
    { lyndon: lyn, necklaces: nec, prenecklaces: pre, cyclic_lyndon: cyl }
  end

  # 長さ n の smooth Lyndon word そのものを列挙する (小さい n 用)
  def words(k, n)
    return [[]] if n.zero?
    out = []
    each_prenecklace(k, n) { |a, t, p| out << a[1..t] if t == n && p == t }
    out
  end

  # ---------------- (B) 分解法 ----------------

  def divisors(n)
    d = []
    i = 1
    while i * i <= n
      if (n % i).zero?
        d << i
        d << n / i unless i == n / i
      end
      i += 1
    end
    d.sort
  end

  def euler_phi(n)
    r = n
    m = n
    d = 2
    while d * d <= m
      if (m % d).zero?
        r -= r / d
        m /= d while (m % d).zero?
      end
      d += 1
    end
    r -= r / m if m > 1
    r
  end

  def mobius(n)
    r = 1
    m = n
    d = 2
    while d * d <= m
      if (m % d).zero?
        m /= d
        return 0 if (m % d).zero?
        r = -r
      end
      d += 1
    end
    m > 1 ? -r : r
  end

  # 転送行列 T[i][j] = [ |i-j| <= 1 ]
  def transfer_matrix(k)
    Array.new(k) { |i| Array.new(k) { |j| (i - j).abs <= 1 ? 1 : 0 } }
  end

  def mat_mul(x, y)
    k = x.size
    Array.new(k) { |i|
      xi = x[i]
      Array.new(k) { |j| (0...k).sum { |l| xi[l] * y[l][j] } }
    }
  end

  def mat_pow(x, e)
    k = x.size
    r = Array.new(k) { |i| Array.new(k) { |j| i == j ? 1 : 0 } }
    b = x
    while e > 0
      r = mat_mul(r, b) if e.odd?
      b = mat_mul(b, b)
      e >>= 1
    end
    r
  end

  def trace_pow(k, d)
    m = mat_pow(transfer_matrix(k), d)
    (0...k).sum { |i| m[i][i] }
  end

  # 巡回的に smooth な Lyndon word 数 D(k,n) と necklace 数 C(k,n)
  def cyclic_counts(k, n)
    return [1, 1] if n.zero?
    tr = divisors(n).to_h { |d| [d, trace_pow(k, d)] }
    d_ = divisors(n).sum { |d| mobius(n / d) * tr[d] } / n
    c_ = divisors(n).sum { |d| euler_phi(n / d) * tr[d] } / n
    [d_, c_]
  end

  # 巻き戻りの辺だけが不良な smooth Lyndon word の個数 B(k,n)。
  # 代表 x[1..n] は x[1] = min かつ x[n] >= x[1] + 2 を満たすので、
  # 先頭は k-3 以下、残り歩数で x[1]+2 に届かない枝は切れる。
  def bad_wrap_count(k, n)
    return 0 if k < 3 || n < 3
    a = Array.new(n + 2, 0)
    cnt = 0
    gen = nil
    gen = lambda { |t, p|
      prev = a[t - 1]
      lo = a[t - p] > prev - 1 ? a[t - p] : prev - 1
      hi = prev + 1 < k - 1 ? prev + 1 : k - 1
      need = a[1] + 2                  # 終端に必要な値
      rest = n - t                     # 残り歩数 (1 歩で高々 +1)
      c = a[t - p]
      if c >= lo && c <= hi && c + rest >= need
        a[t] = c
        # t = n のときこの枝は周期 p < n なので Lyndon word にならない (数えない)
        gen.call(t + 1, p) if t < n
      end
      c = a[t - p] + 1
      c = lo if lo > c
      c = need - rest if need - rest > c
      while c <= hi
        a[t] = c
        if t == n
          cnt += 1 if c >= need        # 周期を破った枝は常に Lyndon word
        else
          gen.call(t + 1, t)
        end
        c += 1
      end
    }
    (0..k - 3).each { |c0| a[1] = c0; gen.call(2, 1) }
    cnt
  end

  # smooth Lyndon word 数と smooth necklace 数を分解法で求める
  def fast(k, n)
    return [1, 1] if n.zero?
    d_, c_ = cyclic_counts(k, n)
    b = bad_wrap_count(k, n)
    [d_ + b, c_ + b]
  end

  # ---------------- 総当たり (検証用) ----------------

  def brute(k, n)
    return [1, 1] if n.zero?
    lyn = 0
    nec = 0
    (k**n).times { |code|
      w = Array.new(n) { |i| (code / k**i) % k }
      next unless (1...n).all? { |i| (w[i] - w[i - 1]).abs <= 1 }
      rots = (0...n).map { |s| w.rotate(s) }
      next unless w == rots.min
      nec += 1
      lyn += 1 if rots.uniq.size == n     # 非周期的なら Lyndon word
    }
    [lyn, nec]
  end

  # ---------------- 表示 ----------------

  SYMBOLS = ['.'] + (1..9).map(&:to_s) + ('a'..'z').to_a

  def to_s_word(w)
    w.map { |c| SYMBOLS[c] }.join
  end
end

# ---------------------------------------------------------------------------
# OEIS 既知項
# ---------------------------------------------------------------------------
KNOWN_LYNDON = {              # smooth Lyndon words (offset 0)
  2 => ['A001037',
        [1, 2, 1, 2, 3, 6, 9, 18, 30, 56, 99, 186, 335, 630, 1161, 2182, 4080,
         7710, 14532, 27594, 52377, 99858, 190557, 364722, 698870, 1342176]],
  3 => ['A215328',
        [1, 3, 2, 5, 10, 24, 49, 112, 240, 534, 1175, 2626, 5848, 13153, 29594,
         66955, 151814, 345494, 788049, 1802675, 4132469, 9495242, 21859912,
         50423465, 116511119, 269666586, 625101288, 1451128164, 3373250909,
         7851415835]]
}.freeze

KNOWN_NECKLACES = {           # smooth necklaces (offset 0)
  3 => ['A215327',
        [1, 3, 5, 8, 15, 27, 58, 115, 252, 541, 1196, 2629, 5894, 13156, 29667,
         66978, 151966, 345497, 788396, 1802678, 4133161, 9495317, 21861393,
         50423468, 116514553, 269666605, 625108573, 1451128479, 3373267275,
         7851415838, 18296568717]],
  4 => ['A215329',
        [1, 4, 7, 12, 25, 51, 121, 272, 656, 1563, 3794, 9193, 22529, 55189,
         136025, 335942, 832605, 2068070, 5150558, 12852754, 32139908, 80509629,
         202016993, 507669052, 1277595853, 3219366640, 8122296152]],
  5 => ['A215331',
        [1, 5, 9, 16, 35, 76, 190, 455, 1156, 2911, 7438, 18992, 48902, 125968,
         325975, 845202, 2197690, 5725854, 14951308, 39110371, 102490649,
         269002564, 707096093, 1861183847, 4905172383, 12942843424]],
  7 => ['A215333',
        [1, 7, 13, 24, 55, 126, 330, 836, 2232, 5926, 15932, 42849, 116011,
         314375, 854952, 2329162, 6359574, 17393334, 47653952, 130752976,
         359275056, 988458426, 2722803495]]
}.freeze

# 巡回的に smooth な Lyndon word (3 色) A215335。offset 1、閉じた式 D(3,n) の検算用。
A215335 = [3, 2, 4, 7, 16, 30, 68, 140, 308, 664, 1476, 3248, 7280, 16286, 36768,
           83160, 189120, 431046, 986244, 2261616, 5200776, 11984382, 27676612,
           64031520, 148406224, 344500520, 800902564, 1864486560, 4346071600,
           10142581552, 23696518916, 55420651440, 129742921992, 304014466080,
           712985901856, 1673486122000].freeze

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
def run_list(k, nmax, all)
  r = SmoothLyndon.counts(k, nmax)
  if all
    puts format('%3s %14s %14s %14s', 'n', 'prenecklaces', 'necklaces', 'Lyndon')
    (0..nmax).each { |n|
      puts format('%3d %14d %14d %14d', n, r[:prenecklaces][n], r[:necklaces][n], r[:lyndon][n])
    }
  else
    (0..nmax).each { |n| puts "#{n} #{r[:lyndon][n]}" }
  end
end

def run_table(kmax, nmax)
  puts('k\\n'.rjust(4) + (0..nmax).map { |n| n.to_s.rjust(12) }.join)
  (1..kmax).each { |k|
    v = SmoothLyndon.counts(k, nmax)[:lyndon]
    puts(k.to_s.rjust(4) + (0..nmax).map { |n| v[n].to_s.rjust(12) }.join)
  }
end

def run_example(k, n)
  w = SmoothLyndon.words(k, n)
  puts "The smooth Lyndon words of length #{n} with #{k} colors " \
       "(using symbols #{SmoothLyndon::SYMBOLS.first(k).map(&:inspect).join(', ')}):"
  w.each_slice(10) { |row| puts row.map { |x| SmoothLyndon.to_s_word(x) }.join(' ') }
  puts "There are #{w.size} smooth Lyndon words, so a(#{n}) = #{w.size}."
end

def run_known(nmax)
  ok = true
  # 1. smooth Lyndon word の既知項 (k = 2, 3)
  KNOWN_LYNDON.each { |k, (name, want)|
    lim = [nmax, want.size - 1].min
    got = SmoothLyndon.counts(k, lim)[:lyndon]
    good = (0..lim).all? { |n| got[n] == want[n] }
    ok &&= good
    puts format('%-4s %s (k=%d): a(0)..a(%d) %s',
                good ? 'OK' : 'NG', name, k, lim, good ? 'matched' : 'MISMATCH')
  }
  # 2. smooth necklace の既知項と  necklaces(n) = L(n) + Sum_{d|n, d<n} D(k,d)
  KNOWN_NECKLACES.each { |k, (name, want)|
    lim = [nmax, want.size - 1].min
    got = SmoothLyndon.counts(k, lim)[:lyndon]
    good = (1..lim).all? { |n|
      s = SmoothLyndon.divisors(n).sum { |d| d == n ? 0 : SmoothLyndon.cyclic_counts(k, d)[0] }
      got[n] + s == want[n]
    }
    ok &&= good
    puts format('%-4s %s (k=%d): L(n) + Sum_{d|n, d<n} D(k,d) for n=1..%d %s',
                good ? 'OK' : 'NG', name, k, lim, good ? 'matched' : 'MISMATCH')
  }
  # 3. 閉じた式 D(3,n) と A215335 (巡回的に smooth な Lyndon word, 3 色, offset 1)
  good = (1..A215335.size).all? { |n| SmoothLyndon.cyclic_counts(3, n)[0] == A215335[n - 1] }
  ok &&= good
  puts format('%-4s A215335 (k=3): D(3,n) for n=1..%d %s',
              good ? 'OK' : 'NG', A215335.size, good ? 'matched' : 'MISMATCH')
  # 4. 内部検算: 生成法 == 分解法  L(n) = D(k,n) + B(k,n), necklaces(n) = C(k,n) + B(k,n)
  (1..6).each { |k|
    lim = [nmax, k <= 2 ? 22 : 12].min
    r = SmoothLyndon.counts(k, lim)
    good = (1..lim).all? { |n|
      d_, c_ = SmoothLyndon.cyclic_counts(k, n)
      b = SmoothLyndon.bad_wrap_count(k, n)
      r[:cyclic_lyndon][n] == d_ && r[:lyndon][n] == d_ + b && r[:necklaces][n] == c_ + b
    }
    ok &&= good
    puts format('%-4s k=%d: n=1..%d  generation == D(k,n)+B(k,n) and C(k,n)+B(k,n)',
                good ? 'OK' : 'NG', k, lim)
  }
  puts ok ? "\nall checks passed." : "\nMISMATCH FOUND."
  ok
end

def run_brute(kmax, nmax)
  ok = true
  (1..kmax).each { |k|
    r = SmoothLyndon.counts(k, nmax)
    (0..nmax).each { |n|
      bl, bn = SmoothLyndon.brute(k, n)
      good = (bl == r[:lyndon][n] && bn == r[:necklaces][n])
      ok &&= good
      puts format('%-4s k=%d n=%2d  Lyndon %d (brute %d)  necklaces %d (brute %d)',
                  good ? 'OK' : 'NG', k, n, r[:lyndon][n], bl, r[:necklaces][n], bn)
    }
  }
  puts ok ? "\nall matched." : "\nMISMATCH FOUND."
  ok
end

if __FILE__ == $PROGRAM_NAME
  nums = ARGV.reject { |s| s.start_with?('--') }.map(&:to_i)
  case ARGV.find { |s| s.start_with?('--') }
  when '--all'     then run_list(nums[0] || 3, nums[1] || 18, true)
  when '--fast'    then (k = nums[0] || 3; n = nums[1] || 20
                         l, a = SmoothLyndon.fast(k, n)
                         puts "k=#{k}  a(#{n}) = #{l}   smooth necklaces = #{a}")
  when '--table'   then run_table(nums[0] || 6, nums[1] || 12)
  when '--example' then run_example(nums[0] || 3, nums[1] || 4)
  when '--known'   then exit(run_known(nums[0] || 16) ? 0 : 1)
  when '--brute'   then exit(run_brute(nums[0] || 4, nums[1] || 8) ? 0 : 1)
  else run_list(nums[0] || 3, nums[1] || 18, false)
  end
end
