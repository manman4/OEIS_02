def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) + s(2, i) - s(3, i) - s(6, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 4 * a[j] * ary[-j]} / i}
  ary
end

n = 32
ary = A(n - 1)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
