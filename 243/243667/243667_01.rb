def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  (-1) ** (n % 2) * (0..n).inject(0){|s, i| s + (-2) ** i * ncr(n, i) * ncr(4 * n + i + 1, n) / (4 * n + i + 1r)}.to_i
end


n = 20
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
