def A(k, n)
  s = 1
  print 0
  print ' '
  puts 1
  (1..n).each{|i|
    s *= k
    print i
    print ' '
    puts s.to_s.size
  }
end

n = 10000
A(5, n)

