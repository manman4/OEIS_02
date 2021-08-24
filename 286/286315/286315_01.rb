require 'prime'

def d(m, k)
  return m unless @factor[k]
  (0..@factor[k][1]).map{|e| d(m * @factor[k][0] ** e, k + 1)}.flatten
end

(0..18).each{|i|
  n = 10 ** i + 1
  # ‡”Ô‚ğ‹t‚É‚·‚é•û‚ªŒã‚ÌŒ‹‰Ê‚ªãY—í‚É‚È‚é
  @factor = Prime.prime_division(n).reverse
  s = 0
  d(1, 0).each{|j|
    s += j * j * j if (n / j) % 2 == 1
  }
  print i
  print ' ' 
  puts s
}
