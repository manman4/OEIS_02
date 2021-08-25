def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  a_ary = Array.new(n + 1, 0)
  (0..n / k).each{|i| a_ary[k * i] = ary[i]}
  a_ary
end

n = 64
u = A(1, n)
v = [0, 0] + A(11, n)[0..-3]
(0..n).each{|i|
  print i
  print ' '
  puts u[i] - v[i]
}
