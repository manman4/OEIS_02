require 'prime'

def pow(a, m, mod)
  return 1 if m == 0
  k = pow(a, m >> 1, mod)
  k *= k
  return k % mod if m & 1 == 0
  return k * a % mod
end


n = 581337017
p pow(5, n, n) - n

n = 7202608727
p pow(5, n, n) - n


[6].each{|i|
  ary = []
  (5551501860..10 ** 10).each{|k|
    p k if k % (10 ** 9) == 0
    l = pow(5, k, k)
    if l == i % k
      p k
      ary << k
    end
  }
  p [i, ary]
}
