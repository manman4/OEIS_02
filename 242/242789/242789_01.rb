def A(n)
  return 3 if n == 2
  cnt = 2
  # cnt - n < 0のときがあることに注意
  while (cnt ** cnt - n) % (cnt - n) != 0
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