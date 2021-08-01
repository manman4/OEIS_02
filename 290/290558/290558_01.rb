require 'OpenSSL'
def f_a(ary, a)
  (0..ary.size - 1).inject(0){|s, i| s + ary[i] * a ** i}
end
def df(ary)
  (1..ary.size - 1).map{|i| i * ary[i]}
end
def A(c_ary, k, m, n)
  x = OpenSSL::BN.new((-f_a(df(c_ary), k)).to_s).mod_inverse(m).to_i % m
  f_ary = c_ary.map{|i| x * i}
  f_ary[1] += 1
  d_ary = []
  ary = [0]
  a, mod = k, m
  (n + 1).times{|i|
    b = a % mod
    d_ary << (b - ary[-1]) / m ** i
    ary << b
    a = f_a(f_ary, b)
    mod *= m
  }
  d_ary
end
def A290558(n)
  A([-2, 0, 1], 4, 7, n)
end
p A290558(100)
