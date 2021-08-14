def pow(a, m, mod)
  return 1 % mod if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A(n)
  m = n * n * n
  s = 0
  (1..n - 1).each{|i|
    s += pow(i, n - 2, m)
  }
  s % m
end

n = 1000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
