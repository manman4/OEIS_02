def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i * i if n % i == 0 && i % k == m}
  s
end
def A(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(2, 1, i) - s(2, 0, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - ary[-j] * s_ary[j]} / i}
  ary
end

n = 39
ary = A(n)
(0..n).each{|i|
  j = (-1) ** (i % 2) * ary[i]
  break if  j.to_s.size > 1000
  print i
  print ' '
  puts j
}
