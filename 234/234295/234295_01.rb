def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = a(n-1) + 4 * Sum_{k=1..n-1} binomial(n-1,k) * a(k) * a(n-k).
def a(n)
  ary = [1]
  (1..n).each{|i|
    ary << (1..i - 1).inject(ary[-1]){|s, k| s + 4 * ncr(i - 1, k) * ary[k] * ary[i - k]}
  }
  ary
end

n = 500
ary = a(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
