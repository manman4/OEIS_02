def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(10, 3, i) + s(10, 4, i) + s(10, 6, i) + s(10, 7, i) - s(10, 1, i) - s(10, 2, i) - s(10, 8, i) - s(10, 9, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 59
ary = A(n + 1)
-1.upto(n){|i|
  print i
  print ' '
  puts ary[i + 1]
}
