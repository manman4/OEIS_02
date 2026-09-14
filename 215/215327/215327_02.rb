#!/usr/bin/env ruby
# frozen_string_literal: true
#
# Smooth necklaces with k colors
#
#   k = 2 : A000031   k = 3 : A215327   k = 4 : A215329   k = 5 : A215331   k = 7 : A215333
#   (smooth Lyndon words: k = 2 : A001037, k = 3 : A215328, k = 4 : A215330,
#                         k = 5 : A215332, k = 7 : A215334)
#   k = 6 は OEIS に見当たらない (1, 6, 11, 20, 45, 101, 260, 645, 1690, 4397, 11590, ...)。
#
# 定義 (A215327 より):
#   necklace (x[1],...,x[n]) が smooth  <=>  |x[k] - x[k-1]| <= 1   (2 <= k <= n)
#   necklace は回転同値類。条件は「辞書式最小の代表」を線形に読んだときのもので、
#   巻き戻りの辺 (x[n], x[1]) は条件に入らない。
#   例: k=3, n=4 の "0012" は smooth necklace だが |2-0| = 2 なので巡回的には smooth でない。
#
# ---------------------------------------------------------------------------
# アルゴリズム
#
# (A) 生成法  … SmoothNecklaces.counts
#     FKM (Fredricksen-Kessler-Maiorana) / Duval の prenecklace 生成木を smooth 条件で
#     枝刈りする。prenecklace a[1..t] の周期を p とすると
#         necklace <=> p | t ,   Lyndon word <=> p = t
#     なので、深さ nmax まで 1 回走査するだけで n = 0..nmax の
#     prenecklace / necklace / Lyndon word 数が同時に得られる (CAT)。
#
# (B) 分解法  … SmoothNecklaces.fast
#     巻き戻りの辺まで smooth な necklace (= 巡回的に smooth) は回転不変な条件なので
#     Burnside で閉じた式になる。T[i][j] = [ |i-j| <= 1 ] (k x k) として
#         C(k,n) = (1/n) * Sum_{d|n} phi(n/d) * tr(T^d)     (巡回 smooth necklace)
#         D(k,n) = (1/n) * Sum_{d|n} mu(n/d)  * tr(T^d)     (巡回 smooth Lyndon word)
#     残りは「不良な辺がちょうど 1 本、しかもそれが巻き戻り位置」の necklace で、
#     これは必ず非周期的なので Lyndon word。その個数を B(k,n) とすると
#         a(n) = C(k,n) + B(k,n),   Lyndon(n) = D(k,n) + B(k,n)
#     B(k,n) だけを生成すればよく (先頭 <= k-3、終端 >= 先頭+2 で強く枝刈りできる)、
#     生成法の約 2 倍速い。C, D 側は閉じた式なので互いの独立な検算にもなる。
#     k=3 では C(3,n) = A208772, D(3,n) = A215335 なので
#         A215327(n) - A208772(n) = A215328(n) - A215335(n) = B(3,n)
#         B(3,n) = 1, 3, 8, 19, 44, 100, 226, 511, 1150, 2600, ...  (n >= 3)
#     (B(k,n) は「x[n] >= x[1]+2 となる smooth Lyndon word」の個数。)
#
# 目安 (Ruby, 手元の環境): 生成法は k=3 で n=20 が約 2 秒、n=22 が約 11 秒。
# 分解法は k=3 で n=22 が約 6 秒、n=24 が約 30 秒。どちらも計算量は a(n) に比例する。
#
# ---------------------------------------------------------------------------
# 使い方
#   ruby 215327_02.rb [K] [NMAX]          # 生成法で n = 0..NMAX の a(n) (既定 K=3, NMAX=18)
#   ruby 215327_02.rb --all [K] [NMAX]    # prenecklace / necklace / Lyndon をまとめて表示
#   ruby 215327_02.rb --fast K N          # 分解法で a(N) のみ
#   ruby 215327_02.rb --table [KMAX] [NMAX]
#   ruby 215327_02.rb --example [K] [N]   # OEIS の EXAMPLE 欄を再現 (既定 K=3, N=4)
#   ruby 215327_02.rb --known [NMAX]      # OEIS 既知項との照合 + 内部検算 (既定 NMAX=16)
#   ruby 215327_02.rb --brute [KMAX] [NMAX]  # 総当たりとの照合

module SmoothNecklaces
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
  #   :necklaces    smooth necklace        (k=3 なら A215327)
  #   :lyndon       smooth Lyndon word     (k=3 なら A215328)
  #   :prenecklaces smooth prenecklace
  #   :cyclic       巻き戻りの辺まで smooth な necklace (検算用)
  def counts(k, nmax)
    nec = Array.new(nmax + 1, 0)
    lyn = Array.new(nmax + 1, 0)
    pre = Array.new(nmax + 1, 0)
    cyc = Array.new(nmax + 1, 0)
    nec[0] = lyn[0] = pre[0] = cyc[0] = 1   # 空語
    each_prenecklace(k, nmax) { |a, t, p|
      pre[t] += 1
      next unless (t % p).zero?             # necklace <=> p | t
      nec[t] += 1
      lyn[t] += 1 if p == t                 # Lyndon   <=> p = t
      cyc[t] += 1 if (a[t] - a[1]).abs <= 1
    }
    { necklaces: nec, lyndon: lyn, prenecklaces: pre, cyclic: cyc }
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

  # 巡回的に smooth な necklace 数 C(k,n) と Lyndon word 数 D(k,n)
  def cyclic_counts(k, n)
    return [1, 1] if n.zero?
    tr = divisors(n).to_h { |d| [d, trace_pow(k, d)] }
    c = divisors(n).sum { |d| euler_phi(n / d) * tr[d] } / n
    l = divisors(n).sum { |d| mobius(n / d) * tr[d] } / n
    [c, l]
  end

  # 巻き戻りの辺だけが不良な smooth necklace (= 必ず Lyndon word) の個数 B(k,n)。
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

  # a(n) と smooth Lyndon word 数を分解法で求める
  def fast(k, n)
    return [1, 1] if n.zero?
    c, l = cyclic_counts(k, n)
    b = bad_wrap_count(k, n)
    [c + b, l + b]
  end

  # ---------------- 総当たり (検証用) ----------------

  def brute(k, n)
    return [1, 1] if n.zero?
    nec = 0
    lyn = 0
    (k**n).times { |code|
      w = Array.new(n) { |i| (code / k**i) % k }
      next unless (1...n).all? { |i| (w[i] - w[i - 1]).abs <= 1 }
      rots = (0...n).map { |s| w.rotate(s) }
      next unless w == rots.min
      nec += 1
      lyn += 1 if rots.uniq.size == n
    }
    [nec, lyn]
  end

  # ---------------- 表示 ----------------

  SYMBOLS = ['.'] + (1..9).map(&:to_s) + ('a'..'z').to_a

  def to_s_word(a, t)
    (1..t).map { |i| SYMBOLS[a[i]] }.join
  end
end

# ---------------------------------------------------------------------------
# OEIS 既知項
# ---------------------------------------------------------------------------
KNOWN = {
  necklaces: {          # smooth necklaces           (offset 0)
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
  },
  lyndon: {             # smooth Lyndon words        (offset 0)
    3 => ['A215328',
          [1, 3, 2, 5, 10, 24, 49, 112, 240, 534, 1175, 2626, 5848, 13153, 29594,
           66955, 151814, 345494, 788049, 1802675, 4132469, 9495242, 21859912,
           50423465, 116511119, 269666586, 625101288, 1451128164, 3373250909,
           7851415835]]
  }
}.freeze

# 巡回的に smooth な Lyndon word (3 色) A215335。offset 1、閉じた式 D(3,n) の検算用。
# 対応する巡回 smooth necklace (3 色) は A208772 = C(3,n)。
A215335 = [3, 2, 4, 7, 16, 30, 68, 140, 308, 664, 1476, 3248, 7280, 16286, 36768,
           83160, 189120, 431046, 986244, 2261616, 5200776, 11984382, 27676612,
           64031520, 148406224, 344500520, 800902564, 1864486560, 4346071600,
           10142581552, 23696518916, 55420651440, 129742921992, 304014466080,
           712985901856, 1673486122000].freeze

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
def run_list(k, nmax, all)
  r = SmoothNecklaces.counts(k, nmax)
  if all
    puts format('%3s %14s %14s %14s', 'n', 'prenecklaces', 'necklaces', 'Lyndon')
    (0..nmax).each { |n|
      puts format('%3d %14d %14d %14d', n, r[:prenecklaces][n], r[:necklaces][n], r[:lyndon][n])
    }
  else
    (0..nmax).each { |n| puts "#{n} #{r[:necklaces][n]}" }
  end
end

def run_table(kmax, nmax)
  rows = (1..kmax).map { |k| [k, SmoothNecklaces.counts(k, nmax)[:necklaces]] }
  puts(['k\\n'.rjust(4)] .concat((0..nmax).map { |n| n.to_s.rjust(12) }).join)
  rows.each { |k, v| puts(k.to_s.rjust(4) + (0..nmax).map { |n| v[n].to_s.rjust(12) }.join) }
end

def run_example(k, n)
  puts "The smooth pre-necklaces, necklaces (N), and Lyndon words (L) of length #{n} " \
       "with #{k} colors (using symbols #{SmoothNecklaces::SYMBOLS.first(k).map(&:inspect).join(', ')}):"
  pre = nec = lyn = 0
  SmoothNecklaces.each_prenecklace(k, n) { |a, t, p|
    next unless t == n
    pre += 1
    mark = +''
    if (t % p).zero?
      nec += 1
      mark << ' N'
      (lyn += 1; mark << ' L') if p == t
    end
    puts "#{SmoothNecklaces.to_s_word(a, n)} #{p} " \
         "#{(n - p + 1..n).map { |i| SmoothNecklaces::SYMBOLS[a[i]] }.join}#{mark}"
  }
  puts "There are #{pre} pre-necklaces, #{nec} necklaces, and #{lyn} Lyndon words."
  puts "So a(#{n}) = #{nec}."
end

def run_known(nmax)
  ok = true
  KNOWN[:necklaces].each_key { |k|
    lim = [nmax, KNOWN[:necklaces][k][1].size - 1].min
    r = SmoothNecklaces.counts(k, lim)
    [[:necklaces, r[:necklaces]], [:lyndon, r[:lyndon]]].each { |kind, got|
      name, want = KNOWN[kind][k]
      next unless name
      good = (0..lim).all? { |n| got[n] == want[n] }
      ok &&= good
      puts format('%-4s %s (k=%d): a(0)..a(%d) %s',
                  good ? 'OK' : 'NG', name, k, lim, good ? 'matched' : 'MISMATCH')
    }
  }
  # 閉じた式 D(3,n) と A215335 (巡回的に smooth な Lyndon word, 3 色, offset 1)
  good = (1..A215335.size).all? { |n| SmoothNecklaces.cyclic_counts(3, n)[1] == A215335[n - 1] }
  ok &&= good
  puts format('%-4s A215335 (k=3): D(3,n) for n=1..%d %s',
              good ? 'OK' : 'NG', A215335.size, good ? 'matched' : 'MISMATCH')
  # 内部検算 1: 生成法で数えた「巡回 smooth」と Burnside の閉じた式
  # 内部検算 2: 生成法の necklace / Lyndon と分解法 a(n) = C + B, L(n) = D + B
  (1..6).each { |k|
    lim = [nmax, k <= 2 ? 22 : 12].min
    r = SmoothNecklaces.counts(k, lim)
    good = (1..lim).all? { |n|
      c, l = SmoothNecklaces.cyclic_counts(k, n)
      b = SmoothNecklaces.bad_wrap_count(k, n)
      r[:cyclic][n] == c && r[:necklaces][n] == c + b && r[:lyndon][n] == l + b
    }
    ok &&= good
    puts format('%-4s k=%d: n=1..%d  generation == C(k,n)+B(k,n) and D(k,n)+B(k,n)',
                good ? 'OK' : 'NG', k, lim)
  }
  puts ok ? "\nall checks passed." : "\nMISMATCH FOUND."
  ok
end

def run_brute(kmax, nmax)
  ok = true
  (1..kmax).each { |k|
    r = SmoothNecklaces.counts(k, nmax)
    (0..nmax).each { |n|
      bn, bl = SmoothNecklaces.brute(k, n)
      good = (bn == r[:necklaces][n] && bl == r[:lyndon][n])
      ok &&= good
      puts format('%-4s k=%d n=%2d  necklaces %d (brute %d)  Lyndon %d (brute %d)',
                  good ? 'OK' : 'NG', k, n, r[:necklaces][n], bn, r[:lyndon][n], bl)
    }
  }
  puts ok ? "\nall matched." : "\nMISMATCH FOUND."
  ok
end

if __FILE__ == $PROGRAM_NAME
  args = ARGV.reject { |s| s.start_with?('--') }
  nums = args.map(&:to_i)
  case ARGV.find { |s| s.start_with?('--') }
  when '--all'     then run_list(nums[0] || 3, nums[1] || 18, true)
  when '--fast'    then (k = nums[0] || 3; n = nums[1] || 20
                         a, l = SmoothNecklaces.fast(k, n)
                         puts "k=#{k}  a(#{n}) = #{a}   smooth Lyndon words = #{l}")
  when '--table'   then run_table(nums[0] || 6, nums[1] || 12)
  when '--example' then run_example(nums[0] || 3, nums[1] || 4)
  when '--known'   then exit(run_known(nums[0] || 16) ? 0 : 1)
  when '--brute'   then exit(run_brute(nums[0] || 4, nums[1] || 8) ? 0 : 1)
  else run_list(nums[0] || 3, nums[1] || 18, false)
  end
end
