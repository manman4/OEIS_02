def A(n)
  -(1..n).inject(0){|s, i| s + (-1) ** ((n / i) % 2)}
end

n = 10000
(0..n).each{|i|
  print i
  print ' '
  puts A(i)
}
