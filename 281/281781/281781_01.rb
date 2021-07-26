def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i * i if n % i == 0 && i % k == m}
  s
end
def A281781(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(2, 1, i) - s(2, 0, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + ary[-j] * s_ary[j]} / i}
  ary
end

n = 50
ary = A281781(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
