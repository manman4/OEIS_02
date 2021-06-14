def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| 6 * s(2, i) + 6 * s(8, i) - 4 * s(1, i) - 4 * s(4, i) - 4 * s(16, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 37
ary = A(n + 1)
-1.upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
