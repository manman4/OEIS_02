def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

n = 10000
(1..n).each{|i|
  j = i ** 8 * phi(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
