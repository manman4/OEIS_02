#!/usr/bin/env ruby
# frozen_string_literal: true
#
# Cyclically smooth Lyndon words with k colors
#
#   k = 2 : A001037   k = 3 : A215335   k = 4 : A215336   k = 5 : A215337   k = 7 : A215338
#   (巡回的に smooth な necklace は A208777 の第 k 列:
#    k = 2 : A000031, k = 3 : A208772, k = 4 : A208773, k = 5 : A208774,
#    k = 6 : A208775, k = 7 : A208776)
#   k = 6 の Lyndon 側は OEIS に見当たらない
#       (6, 5, 10, 19, 46, 96, 236, 547, 1344, 3290, 8272, 20842, 53340, 137175, ...)
#
# 定義 (A215335 より):
#   Lyndon word (x[1],...,x[n]) が smooth  <=>  |x[k] - x[k-1]| <= 1  (2 <= k <= n)
#   さらに cyclically smooth <=>  |x[1] - x[n]| <= 1
#   つまり巻き戻りの辺も込みで隣接差が 1 以下。これは回転不変な条件なので、
#   A215327 / A215328 (巻き戻りを見ない smooth) と違って閉じた式で書ける。
#
# ---------------------------------------------------------------------------
# アルゴリズム (閉じた式)
#
#   転送行列 T[i][j] = [ |i-j| <= 1 ]  (k x k) とすると
#     tr(T^n) = 長さ n の巡回 smooth 文字列の個数 (k=3 なら A124696)
#   Burnside と Moebius 反転で
#     C(k,n) = (1/n) * Sum_{d|n} phi(n/d) * tr(T^d)   巡回 smooth necklace   (A208777 の列 k)
#     a(n) = D(k,n) = (1/n) * Sum_{d|n} mu(n/d) * tr(T^d)   巡回 smooth Lyndon word
#   両者は C(k,n) = Sum_{d|n} D(k,d) で結ばれる (A208772 の FORMULA 欄と同じ)。
#
#   T は道グラフ + ループなので固有値は 1 + 2*cos(i*pi/(k+1)), i = 1..k。つまり
#     tr(T^d) = Sum_{i=1..k} (1 + 2*cos(i*pi/(k+1)))^d
#   で、これが OEIS の Mathematica / PARI 版 (Knopfmacher らの結果) にあたる。
#   本コードは丸め誤差を避けるため整数行列のべき乗で tr(T^d) を出し、
#   固有値版は --eigen で照合できるようにしてある。
#
#   tr(T^1), ..., tr(T^N) は T を 1 回ずつ掛けて求めるので全体 O(N k^3) 回の多倍長乗算。
#   n = 200 程度なら一瞬、n = 2000 でも数秒で厳密値が出る。
#
#   参考: 巻き戻りを見ない smooth 版 (A215327 / A215328) は回転不変でないため
#   閉じた式にならず、生成が要る。両者の差は
#     A215328(n) - A215335(n) = A215327(n) - A208772(n) = 1, 3, 8, 19, 44, ... (n>=3)
#   (= x[n] >= x[1]+2 となる smooth Lyndon word の個数)。
#
# ---------------------------------------------------------------------------
# 使い方
#   ruby 215335_01.rb [K] [NMAX]            # a(n) = D(K,n), n = 1..NMAX (既定 K=3, NMAX=40)
#   ruby 215335_01.rb --necklaces [K] [NMAX]  # 巡回 smooth necklace C(K,n) (A208777 の列 K)
#   ruby 215335_01.rb --table [KMAX] [NMAX]
#   ruby 215335_01.rb --example [K] [N]     # OEIS の EXAMPLE 欄を再現 (既定 K=3, N=4)
#   ruby 215335_01.rb --eigen [K] [NMAX]    # 固有値版 (float) と整数版の照合
#   ruby 215335_01.rb --known [NMAX]        # OEIS 既知項との照合 + 内部検算
#   ruby 215335_01.rb --brute [KMAX] [NMAX] # 総当たりとの照合

module CyclicSmooth
  module_function

  # ---------------- 数論のこまごま ----------------

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

  # ---------------- 閉じた式 ----------------

  # 転送行列 T[i][j] = [ |i-j| <= 1 ]
  def transfer_matrix(k)
    Array.new(k) { |i| Array.new(k) { |j| (i - j).abs <= 1 ? 1 : 0 } }
  end

  # tr(T^d) を d = 1..nmax まで。戻り値は添字 1..nmax (0 番は nil)。
  # T は三重対角なので 1 行あたり高々 3 項しか効かない。
  def traces(k, nmax)
    t = Array.new(nmax + 1)
    return t if nmax < 1
    m = transfer_matrix(k)                    # m = T^d を d = 1 から積み上げる
    d = 1
    loop do
      t[d] = (0...k).sum { |i| m[i][i] }
      break if d == nmax
      m = Array.new(k) { |i|                  # m = m * T
        row = m[i]
        Array.new(k) { |j|
          s = row[j]
          s += row[j - 1] if j > 0
          s += row[j + 1] if j + 1 < k
          s
        }
      }
      d += 1
    end
    t
  end

  # 巡回 smooth な Lyndon word 数 D(k,n) を n = 1..nmax まで (添字 1..nmax)
  def lyndon_counts(k, nmax)
    t = traces(k, nmax)
    a = Array.new(nmax + 1)
    (1..nmax).each { |n| a[n] = divisors(n).sum { |d| mobius(n / d) * t[d] } / n }
    a
  end

  # 巡回 smooth な necklace 数 C(k,n) を n = 1..nmax まで (添字 1..nmax)
  def necklace_counts(k, nmax)
    t = traces(k, nmax)
    a = Array.new(nmax + 1)
    (1..nmax).each { |n| a[n] = divisors(n).sum { |d| euler_phi(n / d) * t[d] } / n }
    a
  end

  def lyndon(k, n)   = n < 1 ? 0 : lyndon_counts(k, n)[n]
  def necklace(k, n) = n < 1 ? 0 : necklace_counts(k, n)[n]

  # 固有値版 (Knopfmacher-Mansour-Munagi-Prodinger; OEIS の Mathematica/PARI と同じ)。
  # tr(T^d) = Sum_{i=1..k} (1 + 2*cos(i*pi/(k+1)))^d を浮動小数で計算して丸める。
  def traces_float(k, nmax)
    ev = (1..k).map { |i| 1 + 2 * Math.cos(i * Math::PI / (k + 1)) }
    t = Array.new(nmax + 1)
    (1..nmax).each { |d| t[d] = ev.sum { |x| x**d }.round }
    t
  end

  def lyndon_counts_float(k, nmax)
    t = traces_float(k, nmax)
    a = Array.new(nmax + 1)
    (1..nmax).each { |n| a[n] = divisors(n).sum { |d| mobius(n / d) * t[d] } / n }
    a
  end

  # ---------------- 生成法 (検証用) ----------------

  # FKM / Duval の prenecklace 生成木を smooth 条件で枝刈りして枚挙する。
  # ブロック引数は (a, t, p): a[1..t] が長さ t、周期 p の smooth prenecklace。
  def each_prenecklace(k, nmax)
    return if k < 1 || nmax < 1
    a = Array.new(nmax + 2, 0)
    gen = nil
    gen = lambda { |t, p|
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

  # 生成で数えた巡回 smooth な Lyndon word 数と necklace 数 (添字 1..nmax)
  def counts_by_generation(k, nmax)
    lyn = Array.new(nmax + 1, 0)
    nec = Array.new(nmax + 1, 0)
    each_prenecklace(k, nmax) { |a, t, p|
      next unless (t % p).zero?             # necklace <=> p | t
      next unless (a[t] - a[1]).abs <= 1    # 巻き戻りの辺も smooth
      nec[t] += 1
      lyn[t] += 1 if p == t                 # Lyndon <=> p = t
    }
    { lyndon: lyn, necklaces: nec }
  end

  # 長さ n の巡回 smooth な Lyndon word そのもの (小さい n 用)
  def words(k, n)
    out = []
    each_prenecklace(k, n) { |a, t, p|
      out << a[1..t] if t == n && p == t && (a[t] - a[1]).abs <= 1
    }
    out
  end

  # ---------------- 総当たり (検証用) ----------------

  def brute(k, n)
    lyn = 0
    nec = 0
    (k**n).times { |code|
      w = Array.new(n) { |i| (code / k**i) % k }
      next unless (0...n).all? { |i| (w[i] - w[i - 1]).abs <= 1 }   # i=0 で巻き戻りも見る
      rots = (0...n).map { |s| w.rotate(s) }
      next unless w == rots.min
      nec += 1
      lyn += 1 if rots.uniq.size == n
    }
    [lyn, nec]
  end

  # ---------------- 表示 ----------------

  SYMBOLS = ['.'] + (1..9).map(&:to_s) + ('a'..'z').to_a

  def to_s_word(w) = w.map { |c| SYMBOLS[c] }.join
end

# ---------------------------------------------------------------------------
# OEIS 既知項
# ---------------------------------------------------------------------------

# A215335 (k=3, offset 1) の a(1..40) と b-file の抜き取り
A215335 = [3, 2, 4, 7, 16, 30, 68, 140, 308, 664, 1476, 3248, 7280, 16286, 36768,
           83160, 189120, 431046, 986244, 2261616, 5200776, 11984382, 27676612,
           64031520, 148406224, 344500520, 800902564, 1864486560, 4346071600,
           10142581552, 23696518916, 55420651440, 129742921992, 304014466080,
           712985901856, 1673486122000, 3930960079760, 9240435140958,
           21736375155040, 51164344278432].freeze
A215335_SPOT = {
  50 => 275305103619334456,
  100 => 1894822502992738668219809046570779184,
  150 => 17388476855575485758433786881873203897662386752049704048,
  200 => 179517615892383356990199864810938774471367623549588194326534840615260578848
}.freeze

# A208772 (k=3 の巡回 smooth necklace, offset 1)
A208772 = [3, 5, 7, 12, 19, 39, 71, 152, 315, 685, 1479, 3294, 7283, 16359, 36791,
           83312, 189123, 431393, 986247, 2262308, 5200851, 11985863, 27676615,
           64034954, 148406243, 344507805, 800902879, 1864502926, 4346071603,
           10142619039, 23696518919, 55420734752, 129742923475, 304014655205,
           712985901943, 1673486556648].freeze

# A208777 の表 (行 n = 1..8, 列 k = 1..18)。巡回 smooth necklace C(k,n)。
A208777 = [
  [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18],
  [1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31, 33, 35],
  [1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 49, 52],
  [1, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 96, 102],
  [1, 8, 19, 30, 41, 52, 63, 74, 85, 96, 107, 118, 129, 140, 151, 162, 173, 184],
  [1, 14, 39, 65, 91, 117, 143, 169, 195, 221, 247, 273, 299, 325, 351, 377, 403, 429],
  [1, 20, 71, 128, 185, 242, 299, 356, 413, 470, 527, 584, 641, 698, 755, 812, 869, 926],
  [1, 36, 152, 293, 435, 577, 719, 861, 1003, 1145, 1287, 1429, 1571, 1713, 1855, 1997,
   2139, 2281]
].freeze

# A001037 (2 色の Lyndon word; 2 色では常に巡回 smooth), offset 0
A001037 = [1, 2, 1, 2, 3, 6, 9, 18, 30, 56, 99, 186, 335, 630, 1161, 2182, 4080,
           7710, 14532, 27594, 52377, 99858, 190557, 364722, 698870, 1342176].freeze

# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------
def run_list(k, nmax, which)
  v = which == :necklaces ? CyclicSmooth.necklace_counts(k, nmax) : CyclicSmooth.lyndon_counts(k, nmax)
  (1..nmax).each { |n| puts "#{n} #{v[n]}" }
end

def run_table(kmax, nmax)
  w = [12, CyclicSmooth.lyndon(kmax, nmax).to_s.size + 2].max
  puts('k\\n'.rjust(4) + (1..nmax).map { |n| n.to_s.rjust(w) }.join)
  (1..kmax).each { |k|
    v = CyclicSmooth.lyndon_counts(k, nmax)
    puts(k.to_s.rjust(4) + (1..nmax).map { |n| v[n].to_s.rjust(w) }.join)
  }
end

def run_example(k, n)
  puts "The cyclically smooth necklaces (N) and Lyndon words (L) of length #{n} " \
       "with #{k} colors (using symbols #{CyclicSmooth::SYMBOLS.first(k).map(&:inspect).join(', ')}):"
  nec = lyn = 0
  CyclicSmooth.each_prenecklace(k, n) { |a, t, p|
    next unless t == n && (t % p).zero? && (a[t] - a[1]).abs <= 1
    nec += 1
    mark = p == t ? (lyn += 1; ' N L') : ' N'
    puts "#{CyclicSmooth.to_s_word(a[1..n])} #{p} " \
         "#{CyclicSmooth.to_s_word(a[n - p + 1..n])}#{mark}"
  }
  puts "There are #{nec} necklaces and a(#{n}) = #{lyn} Lyndon words."
end

def run_eigen(k, nmax)
  exact = CyclicSmooth.lyndon_counts(k, nmax)
  appr = CyclicSmooth.lyndon_counts_float(k, nmax)
  bad = (1..nmax).reject { |n| exact[n] == appr[n] }
  (1..nmax).each { |n| puts format('%4d %20s %20s %s', n, exact[n], appr[n], exact[n] == appr[n] ? '' : '<- differs') }
  puts bad.empty? ? "\ninteger matrix == eigenvalue formula for n = 1..#{nmax}." :
                    "\nfloat version breaks down at n = #{bad.first} (double の桁落ち)."
end

def run_known(nmax)
  ok = true
  check = lambda { |name, good, note|
    ok &&= good
    puts format('%-4s %s: %s', good ? 'OK' : 'NG', name, note)
  }

  # 1. A215335 (k=3) 本体と b-file の抜き取り
  v = CyclicSmooth.lyndon_counts(3, 200)
  check.call('A215335 (k=3)', (1..A215335.size).all? { |n| v[n] == A215335[n - 1] },
             "a(1)..a(#{A215335.size}) matched")
  check.call('A215335 b-file', A215335_SPOT.all? { |n, want| v[n] == want },
             "n = #{A215335_SPOT.keys.join(', ')} matched")

  # 2. A208772 (k=3 の巡回 smooth necklace) と C = Sum_{d|n} D
  c = CyclicSmooth.necklace_counts(3, A208772.size)
  check.call('A208772 (k=3)', (1..A208772.size).all? { |n| c[n] == A208772[n - 1] },
             "a(1)..a(#{A208772.size}) matched")
  check.call('C(k,n) = Sum_{d|n} D(k,d)',
             (1..6).all? { |k|
               cc = CyclicSmooth.necklace_counts(k, 60)
               dd = CyclicSmooth.lyndon_counts(k, 60)
               (1..60).all? { |n| cc[n] == CyclicSmooth.divisors(n).sum { |d| dd[d] } }
             }, 'k = 1..6, n = 1..60')

  # 3. A208777 の表 (行 n = 1..8, 列 k = 1..18)
  check.call('A208777 (table)',
             A208777.each_with_index.all? { |row, i|
               row.each_with_index.all? { |want, j| CyclicSmooth.necklace(j + 1, i + 1) == want }
             }, 'n = 1..8, k = 1..18 matched')

  # 4. 2 色: 巡回 smooth は自明に成立するので A001037 / A000031 (= A208777 の列 2)
  v2 = CyclicSmooth.lyndon_counts(2, A001037.size - 1)
  check.call('A001037 (k=2)', (1..A001037.size - 1).all? { |n| v2[n] == A001037[n] },
             "a(1)..a(#{A001037.size - 1}) matched")

  # 5. 生成法との照合
  lim = [nmax, 14].min
  good = (1..7).all? { |k|
    g = CyclicSmooth.counts_by_generation(k, lim)
    d_ = CyclicSmooth.lyndon_counts(k, lim)
    c_ = CyclicSmooth.necklace_counts(k, lim)
    (1..lim).all? { |n| g[:lyndon][n] == d_[n] && g[:necklaces][n] == c_[n] }
  }
  check.call('generation', good, "k = 1..7, n = 1..#{lim}")

  # 6. 固有値版 (float) との照合
  good = (2..6).all? { |k|
    e = CyclicSmooth.lyndon_counts(k, 20)
    f = CyclicSmooth.lyndon_counts_float(k, 20)
    (1..20).all? { |n| e[n] == f[n] }
  }
  check.call('eigenvalue formula', good, 'k = 2..6, n = 1..20')

  puts ok ? "\nall checks passed." : "\nMISMATCH FOUND."
  ok
end

def run_brute(kmax, nmax)
  ok = true
  (1..kmax).each { |k|
    d_ = CyclicSmooth.lyndon_counts(k, nmax)
    c_ = CyclicSmooth.necklace_counts(k, nmax)
    (1..nmax).each { |n|
      bl, bn = CyclicSmooth.brute(k, n)
      good = (bl == d_[n] && bn == c_[n])
      ok &&= good
      puts format('%-4s k=%d n=%2d  Lyndon %d (brute %d)  necklaces %d (brute %d)',
                  good ? 'OK' : 'NG', k, n, d_[n], bl, c_[n], bn)
    }
  }
  puts ok ? "\nall matched." : "\nMISMATCH FOUND."
  ok
end

if __FILE__ == $PROGRAM_NAME
  nums = ARGV.reject { |s| s.start_with?('--') }.map(&:to_i)
  case ARGV.find { |s| s.start_with?('--') }
  when '--necklaces' then run_list(nums[0] || 3, nums[1] || 40, :necklaces)
  when '--table'     then run_table(nums[0] || 7, nums[1] || 12)
  when '--example'   then run_example(nums[0] || 3, nums[1] || 4)
  when '--eigen'     then run_eigen(nums[0] || 3, nums[1] || 40)
  when '--known'     then exit(run_known(nums[0] || 14) ? 0 : 1)
  when '--brute'     then exit(run_brute(nums[0] || 4, nums[1] || 8) ? 0 : 1)
  else run_list(nums[0] || 3, nums[1] || 40, :lyndon)
  end
end
