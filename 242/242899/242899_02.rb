def pow(a, m, mod)
  return 1 % mod if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

def A(n)
  cnt = 2
  while (pow(n, cnt, n + cnt) + pow(cnt, n, n + cnt)) % (n + cnt) > 0
    cnt += 1
  end
  cnt
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}