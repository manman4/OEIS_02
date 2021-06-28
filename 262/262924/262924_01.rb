def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i * i if n % i == 0 && i % k == m}
  s
end
def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, 0, i) - s(1, 0, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - ary[-j] * s_ary[j]} / i}
  ary
end

n = 41
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
