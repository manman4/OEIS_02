def A225407(n)
  ary = [3]
  a = 3
  n.times{|i|
    b = (a + 7 * (a ** 3 + 3)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

p A225407(100)
