def A225408(n)
  ary = [7]
  a = 7
  n.times{|i|
    b = (a + 7 * (a ** 3 + 7)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

p A225408(100)
