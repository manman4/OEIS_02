#!/usr/bin/env ruby
# frozen_string_literal: true
#
# A288676: Numbers k such that A108394(k) is not a prime.
#
#   A108394 ... k-Goebel 数列 f(0)=1, f(n)=(1+f(0)^n+...+f(n-1)^n)/n が
#               整数でなくなる最初の添字。OEIS でのオフセットは n=2。
#
# 使い方:
#   ruby a288676.rb [b108394.txt] [--list] [--limit N]
#
#   --list     末尾にカンマ区切りの一覧を表示
#   --limit N  b-file の n <= N の行だけを対象にする
#
# 出力: 標準出力に A288676 の b-file 形式 ("1 5", "2 22", ...)
#       進捗や集計は標準エラーへ。

require 'openssl'

# --- 小さい素数（試し割り用） -------------------------------------------
SMALL_PRIMES = begin
  limit = 1000
  sieve = Array.new(limit + 1, true)
  sieve[0] = sieve[1] = false
  (2..Integer.sqrt(limit)).each do |i|
    next unless sieve[i]
    (i * i).step(limit, i) { |j| sieve[j] = false }
  end
  (2..limit).select { |i| sieve[i] }.freeze
end
SMALL_LIMIT = SMALL_PRIMES.last**2  # ここ未満なら試し割りだけで確定

# --- 素数判定 -----------------------------------------------------------
# 小さい数は試し割りで確定、大きい数は OpenSSL の Miller-Rabin。
# (stdlib の 'prime' でもよいが、Prime.prime? は試し割りなので大きい値で遅い)
def prime?(n)
  return false if n < 2

  SMALL_PRIMES.each do |p|
    return true  if n == p
    return false if (n % p).zero?
  end
  return true if n < SMALL_LIMIT

  OpenSSL::BN.new(n).prime?
end

# --- b-file の読み込み --------------------------------------------------
# 形式は "n a(n)"。'#' 以降はコメント、空行は無視。
def each_bfile_entry(path)
  return to_enum(:each_bfile_entry, path) unless block_given?

  File.foreach(path).with_index(1) do |line, lineno|
    body = line.sub(/#.*\z/m, '').strip
    next if body.empty?

    n, a = body.split(/\s+/, 3)
    unless n =~ /\A-?\d+\z/ && a =~ /\A-?\d+\z/
      warn "warning: #{path}:#{lineno}: 解釈できない行をスキップ: #{line.chomp}"
      next
    end
    yield n.to_i, a.to_i
  end
end

# --- main ---------------------------------------------------------------
args  = ARGV.dup
list  = args.delete('--list')
limit = (i = args.index('--limit')) ? args.slice!(i, 2)[1].to_i : nil
path  = args.shift || 'b108394.txt'

abort "#{path} が見つかりません" unless File.exist?(path)

found   = []
checked = 0
maxn    = 0

each_bfile_entry(path) do |n, a|
  next if limit && n > limit

  checked += 1
  maxn = n if n > maxn
  next if prime?(a)

  found << n
  puts "#{found.size} #{n}"
  $stdout.flush
  warn format('  A108394(%d) = %d は素数でない', n, a)
end

warn "---"
warn "#{checked} 項を判定 (n <= #{maxn})、非素数は #{found.size} 個"
warn "次の項 a(#{found.size + 1}) > #{maxn}" if found.any?
puts found.join(', ') if list
