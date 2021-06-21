def f(n)
  return n - 1 if n <= 2
  return n / 2 + f(n / 2) if n % 2 == 0
  return 3 * n + 1 + f(3 * n + 1)
end

(1..650289).each{|i|
  print i
  print ' '
  puts f(i)
}
