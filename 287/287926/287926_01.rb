def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

n = 66
ary = [[49, 1], [1, -1]]
(1..n).each{|i|
  print i
  print ' '
  puts ary.inject(0){|s, j| s - j[1] * s(j[0], i)}
}
