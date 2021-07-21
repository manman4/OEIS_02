def A272690(n)
  22 * (0..n - 2).inject(0){|s, i| s + 46 ** i * 2 ** (n - 2 - i)} + 2 ** (n - 1)
end

n = 100
(1..n).each{|i|
  j = A272690(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
