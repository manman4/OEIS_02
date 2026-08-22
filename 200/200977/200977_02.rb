#!/usr/bin/env ruby
# frozen_string_literal: true
#
# A200977
#   a(n) is the smallest nonnegative integer k where exactly n pairs of
#   positive integers (x, y) exist such that x^2 + 2*y^2 = k.
#   （a(0) = 0 を前置した添字）
#
# ------------------------------------------------------------------
# 理論（判別式 -8 は基本判別式。Z[sqrt(-2)] は最大整環かつ PID なので
#       2 進の補正因子は一切現れない。x^2+3y^2 や x^2+7y^2 と違う点）
# ------------------------------------------------------------------
#   k = 2^a * Π p_i^e_i * Π q_j^f_j
#       p_i ≡ 1, 3 (mod 8) : 分解素数（3, 11, 17, 19, 41, 43, ...）
#       q_j ≡ 5, 7 (mod 8) : 惰性素数
#
#   D(k) := Π(e_i + 1)   ... f_j が全て偶数のとき
#         := 0           ... それ以外
#   （2 の指数 a は D に効かない。2 は分岐するので選択の余地がない）
#
#   Z[sqrt(-2)] はノルムに関してユークリッド整域（(1/4) + 2*(1/4) = 3/4 < 1）
#   で単数は ±1 のみ。よって符号込み整数解の総数は分岐なしで
#       T(k) = 2 * D(k)
#
#   退化解は y=0 (k が平方) と x=0 (k が2倍平方) の 2 種で、それぞれ符号込み
#   2 個。平方数の 2 進付値は偶、2倍平方数の 2 進付値は奇なので両立しない。
#       A(k) := [k は平方] + [k は2倍平方] ∈ {0, 1}
#       T(k) = 4*R(k) + 2*A(k)
#   したがって
#       R(k) = (D(k) - A(k)) / 2
#
#   ここで A(k) = D(k) mod 2 が自動的に成り立つ:
#       D が奇 <=> 全 e_i が偶 <=> （表現可能なら）全奇素数指数が偶
#              <=> k は平方数または2倍平方数（2 の指数の偶奇で決まる）
#   よって
#       R(k) = floor(D(k) / 2)
#
#   したがって R(k) = n (n >= 1) となる最小の k は
#       a(n) = min( minprod(2n), minprod(2n+1) )
#   の 2 候補を比べるだけでよい。2^a や q^(偶数) を掛けても D は変わらず
#   k が増えるだけなので、純粋な分解素数部分の最小値がそのまま答になる。
#
#   minprod(M) := 分解素数 3, 11, 17, 19, 41, ... のみを使って
#                 Π(e_i + 1) = M となる最小の数
#
#   a(0) = 0（k = 0 に正の組はない）。
#
#   系: すべての n について 3 | a(n)。
#       M >= 2 なら minprod(M) は最小の分解素数 3 を必ず含む（交換論法により
#       最大の指数が最小素数に割り当てられる）。n >= 1 では 2n >= 2 かつ
#       2n+1 >= 3 なので両候補とも 3 の倍数。a(0) = 0 も 3 の倍数。
#
# 使い方:
#   ruby 200977_02.rb           # n = 0..100 を出力
#   ruby 200977_02.rb 30        # n = 0..30 を出力
#   ruby 200977_02.rb --known   # 総当たりで確定済みの項と照合
#   ruby 200977_02.rb --verify  # 総当たりと突き合わせて理論式を検証

module A200977
  module_function

  SPLIT_RES = [1, 3].freeze

  # ---------- 素数まわり（prime gem に依存しない） ----------

  def prime?(n)
    return false if n < 2
    return true  if n < 4
    return false if n.even?

    d = 3
    while d * d <= n
      return false if (n % d).zero?
      d += 2
    end
    true
  end

  # p ≡ 1, 3 (mod 8) な奇素数を小さい順に count 個
  def primes_split(count)
    ps = []
    n = 3
    while ps.size < count
      ps << n if SPLIT_RES.include?(n % 8) && prime?(n)
      n += 2
    end
    ps
  end

  # ---------- minprod ----------

  # Π(e_i + 1) = m となる最小の Π primes[i]^e_i
  # 指数は非増加としてよい（小さい素数に大きい指数を割り当てるのが最適）
  def minprod(m, primes = nil, idx = 0, max_exp = Float::INFINITY, memo = {})
    return 1 if m == 1

    primes ||= primes_split(Math.log2(m).floor + 1)
    key = [m, idx, max_exp]
    cached = memo[key]
    return cached if cached
    return nil if idx >= primes.size

    best = nil
    divisors_from(m, 2).each { |d|
      e = d - 1
      next if e > max_exp

      rest = minprod(m / d, primes, idx + 1, e, memo)
      next if rest.nil?

      cand = primes[idx]**e * rest
      best = cand if best.nil? || cand < best
    }
    memo[key] = best
  end

  def divisors_from(m, lower)
    ds = []
    d = 1
    while d * d <= m
      if (m % d).zero?
        ds << d if d >= lower
        e = m / d
        ds << e if e != d && e >= lower
      end
      d += 1
    end
    ds.sort
  end

  # ---------- 本体 ----------

  def a(n)
    return 0 if n.zero?

    cands = []
    [2 * n, 2 * n + 1].each { |m|
      v = minprod(m)
      cands << v if v
    }
    cands.min
  end

  def sequence(nmax)
    (0..nmax).map { |n| a(n) }
  end

  # 独立に確定している値（n => a(n)）
  #   n <= 32 の大半は k <= 7*10^7 の総当たり走査で確定したもの。
  #   a(23), a(29) はその範囲外だが、OEIS 側で訂正された値。
  KNOWN = {
    0 => 0, 1 => 3, 2 => 27, 3 => 99, 4 => 297, 5 => 891, 6 => 1683,
    7 => 8019, 8 => 5049, 9 => 18513, 10 => 15147, 11 => 649539,
    12 => 31977, 13 => 314721, 14 => 136323, 15 => 166617, 16 => 95931,
    17 => 10673289, 18 => 351747, 19 => 64304361, 20 => 287793,
    21 => 1499553, 22 => 2832489, 23 => 345191655699, 24 => 863379,
    25 => 20160657, 27 => 5979699, 28 => 2590137,
    29 => 251644717004571, 30 => 3165723, 31 => 25492401, 32 => 3933171
  }.freeze

  def check_known
    ok = true
    KNOWN.keys.sort.each { |n|
      want = KNOWN[n]
      got = a(n)
      good = (got == want)
      ok &&= good
      puts format('%-4s n=%-3d got=%-16d known=%d', good ? 'OK' : 'NG', n, got, want)
    }
    puts ok ? "\nmatches all #{KNOWN.size} known terms." : "\nMISMATCH."
    ok
  end

  # ---------- 検証用の総当たり ----------

  # x^2 + 2*y^2 = k となる正整数の組の個数
  def count_pairs(k)
    cnt = 0
    y = 1
    while 2 * y * y < k
      r = k - 2 * y * y
      s = Integer.sqrt(r)
      cnt += 1 if s * s == r && s.positive?
      y += 1
    end
    cnt
  end

  # k <= limit を総当たりして first[n] を作り、理論値と突き合わせる。
  # 理論値が limit を超える n は「その範囲に現れないこと」だけを確認する。
  def verify(limit = 5_000_000, nmax = 40)
    warn "brute force scan up to k = #{limit} ..."
    cnt = Array.new(limit + 1, 0)
    y = 1
    while 2 * y * y <= limit
      x = 1
      while x * x + 2 * y * y <= limit
        cnt[x * x + 2 * y * y] += 1
        x += 1
      end
      y += 1
    end

    first = {}
    cnt.each_with_index { |c, k| first[c] ||= k }
    first[0] = 0 # k = 0 に正の組はない

    ok = true
    (0..nmax).each { |n|
      theory = a(n)
      brute  = first[n]

      if theory <= limit
        good = (brute == theory)
        puts format('%-4s n=%-4d theory=%-16d brute=%s',
                    good ? 'OK' : 'NG', n, theory, brute.inspect)
      else
        good = brute.nil?
        puts format('%-4s n=%-4d theory=%-16d (> limit, brute=%s)',
                    good ? 'skip' : 'NG', n, theory, brute.inspect)
      end
      ok &&= good
    }
    puts ok ? "\nall matched." : "\nMISMATCH FOUND."
    ok
  end
end

if __FILE__ == $PROGRAM_NAME
  if ARGV.include?('--known')
    A200977.check_known
  elsif ARGV.include?('--verify')
    nums = ARGV.select { |s| s =~ /\A\d+\z/ }.map(&:to_i)
    A200977.verify(nums[0] || 5_000_000, nums[1] || 40)
  else
    nmax = (ARGV[0] || 100).to_i
    A200977.sequence(nmax).each_with_index { |v, n| puts "#{n} #{v}" }
  end
end
