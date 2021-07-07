def A(k, n)
  s = 0
  (1..n).each{|i|
    s += [i, n / i].min ** k if n % i == 0
  }
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(3, i)
}
