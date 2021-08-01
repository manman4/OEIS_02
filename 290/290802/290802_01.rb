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
  ary = [0]
  a, mod = k, m
  (n + 1).times{|i|
    b = a % mod
    ary << b
    a = f_a(f_ary, b)
    mod *= m
  }
  ary
end

n = 21
ary = A([6, 0, 1], 6, 7, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
