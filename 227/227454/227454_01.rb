def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def B(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 24 / (k - 1) * a[j] * ary[-j]} / i}
  ary
end

n = 34
ary = B(9, n - 1)
(1..n).each{|i|
  print i
  print ' '
  puts (-1) ** (i % 2 + 1) * ary[i - 1]
}
