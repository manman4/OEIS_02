require 'prime'

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A(m, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - m * a[j] * ary[-j]} / i}
  ary
end

def B(n)
  p_ary = [0] + Prime.take(n).to_a
  m = p_ary[-1]
  ary = [0] + A(24, m)
  ary1 = (0..m).map{|i| 462 * i ** 6 + 330 * i ** 4 - 165 * i ** 3 + 55 * i ** 2 - 11 * i + 1}
  (1..n).map{|i| ary[p_ary[i]] + ary1[p_ary[i]]}
end

n = 19
ary = B(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
