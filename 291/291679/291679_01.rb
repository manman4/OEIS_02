def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + k * a[j] * ary[-j]} / i}
  ary
end

n = 29
(0..n).each{|i|
  j = A(i, i)[-1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
