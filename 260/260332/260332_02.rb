def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# Conjecture: a(n) = Sum_{k=1..n} binomial(n,k)*binomial(4*n,k-1)*2^k/n for n > 0.
def a260332(n)
  ary = [1]
  (1..n).each{|i|
    s = 0
    (1..i).each{|k|
      s += ncr(i, k) * ncr(4*i, k-1) * 2**k
    }
    ary << s / i
  }
  ary
end

n = 800
ary = a260332(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}