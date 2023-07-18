def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(n) = Sum_{k=1..n-1} (-1)^[(n-k-1)/2] * binomial(n,k) * k^(2*n-2*k) * a(k) for n>1 with a(0)=a(1)=1.
def A(n)
  ary = [1, 1]
  (2..n).each{|i| ary << (1..i - 1).inject(0){|s, j| s + (-1) ** ((i - j - 1) / 2) * ncr(i, j) * j ** (2 * i - 2 * j) * ary[j]}}
  ary
end

n = 200
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
