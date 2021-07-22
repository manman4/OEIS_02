def pow(a, m, mod)
  return 1 % mod if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end

ary = []
(1..9816013).each{|i|
  ary << i if pow(3, i, i) == (i - 11) % i
}
p ary
