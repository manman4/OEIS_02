def A(n)
  return 1 if n == 0
  (1..n).inject(1){|s, i| s + (n - i + 1..n).inject(:*) ** i}
end

n = 10
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
