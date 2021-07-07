def A(n)
  -462 * n ** 6 - 330 * n ** 4 + 165 * n ** 3 - 55 * n ** 2 + 11 * n - 1
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts -A(i)
}
