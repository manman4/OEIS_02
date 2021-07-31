def s(n)
  s = 0
  (1..n).each{|i| s += i ** (1 + n) if n % i == 0}
  s
end

n = 400
(1..n).each{|i|
  j = s(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
