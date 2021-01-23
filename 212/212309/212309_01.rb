def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

n = 1000
(0..n).each{|i|
  j = f(i) % (3 ** i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}