def A001876(n)
  return 0 if n == 0
  s = 0
  (1..n).each{|i|
    s += 1 if n % i == 0 && i % 5 == 1
  }
  s
end

n = 10000
s = 0
(0..n).each{|i| 
  s += A001876(i)
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
