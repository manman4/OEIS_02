def A(k, n)
  s = 0
  (1..n).each{|i|
    s += i if n % i == 0 && (i % k == 0 || i % k == 1 || i % k == k - 1)
  }
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(7, i)
}
