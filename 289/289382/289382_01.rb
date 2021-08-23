def A(n)
  m = n * (n + 1) / 2
  s = 1
  (1..n).each{|i|
    s *= 2
    s %= m
  }
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
