def A(n)
  cnt = 2
  while (n ** cnt + cnt ** n) % (n + cnt) > 0
    cnt += 1
  end
  cnt
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}