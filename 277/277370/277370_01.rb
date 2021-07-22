def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

ary = []
(1..10 ** 9).each{|k|
  p k if k % (10 ** 8) == 0
  l = pow(7, k, k)
  if (l + 2) % k == 0
    p k
    ary << k
  end
  }
p ary
