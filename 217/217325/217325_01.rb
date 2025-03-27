def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def C(n)
  ncr(2 * n, n) / (n + 1)
end

# a(n) = C(floor(n/2 + 1/2))*C(floor(n/2 + 1)) 
def a005817(n)
  C((n + 1) / 2) * C(n / 2 + 1)
end

# a(n) = 6 * n! * Sum_{k=0..floor(n/2)} (2*k+2)!/((n-2*k)!*k!*(k+1)!*(k+2)!*(k+3)!).
def a049401(n)
  (6 * f(n) * (0..n / 2).inject(0){|s, k| s + f(2 * k + 2) / (f(n - 2 * k) * f(k) * f(k + 1) * f(k + 2) * f(k + 3).to_r)}).to_i
end

# A049401(n)-A005817(n)
def a(n)
  a049401(n) - a005817(n)
end

n = 1000
(5..n).each{|i|
  j = a(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}