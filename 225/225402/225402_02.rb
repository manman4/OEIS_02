def A225402(n)
  ary = [7]
  a = 7
  n.times{|i|
    b = (a + 9 * (3 * a ** 3 + 1)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

p A225402(100)
