def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def t(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 5 * (s(5, 1, i) + s(5, 4, i)) - 2 * t(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 83
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
