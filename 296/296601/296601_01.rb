def A(k, n)
  s = 0
  (1..n).each{|i|
    s += i ** (n / i + k) if n % i == 0
  }
  s
end

n = 5000
(1..n).each{|i|
  j = A(2, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
