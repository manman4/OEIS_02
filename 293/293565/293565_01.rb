def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(n) = n! * Sum_{k=0..floor(n/3)} (-1)^(n-k) * binomial(n-2*k-1,k)/(n-3*k)!.
def A(n)
  m = f(n)
  (0..n / 3).inject(0){|s, i| s + (-1) ** (n - i) * ncr(n - 2 * i - 1, i) * m / f(n - 3 * i)}
end

n = 1000
(0..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}