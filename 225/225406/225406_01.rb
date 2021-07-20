def A225406(n)
  ary = [9]
  a = 9
  n.times{|i|
    b = (a + 3 * (a ** 3 - 9)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

p A225406(100)
