def A225464(n)
  ary = [9]
  a = 9
  n.times{|i|
    b = (a +  9 * (9 * a ** 9 - 1)) % (10 ** (i + 2))
    ary << (b - a) / (10 ** (i + 1))
    a = b
  }
  ary
end

p A225464(100)
