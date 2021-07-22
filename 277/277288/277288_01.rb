def pow(a, m, mod)
  return 1 % mod if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

ary = []
(1..244330711).each{|i|
  ary << i if pow(3, i, i) == (i - 5) % i
}
p ary
