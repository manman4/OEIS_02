def f(n) 
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(n) = Sum_{k=0..n} (2*k)! * (n+2*k)!/(k!^5 * (n-k)!).
def a(n)
  (0..n).inject(0){|s, k| s + f(2*k) * f(n+2*k) / (f(k)**5 * f(n-k))}
end

n = 488
(0..n).each{|i| 
  print i
  print ' '
  puts a(i)
}
