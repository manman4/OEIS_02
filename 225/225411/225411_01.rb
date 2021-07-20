def A225411(n)
  ary = [3]
  a = 3
  n.times{|i|
    b = (a + 9 * (3 * a ** 3 - 1)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

p A225411(100)
