def A059415(n)
  i = 0
  a, b = 0, 6
  ary = [0]
  while i < n
    i += 1
    a, b = b, ((((34 * i + 51) * i + 27) * i + 5) * b - i ** 3 * a) / (i + 1r) ** 3
    ary << a.numerator
  end
  ary
end

def A005259(n)
  i = 0
  a, b = 1, 5
  ary = [1]
  while i < n
    i += 1
    a, b = b, ((((34 * i + 51) * i + 27) * i + 5) * b - i ** 3 * a) / (i + 1) ** 3
    ary << a
  end
  ary
end

def A059416(n)
  i = 0
  a, b = 0, 6
  ary = [1]
  while i < n
    i += 1
    a, b = b, ((((34 * i + 51) * i + 27) * i + 5) * b - i ** 3 * a) / (i + 1r) ** 3
    ary << a.denominator
  end
  ary
end

n = 1000
ary0 = A059415(n)
ary1 = A005259(n)
ary2 = A059416(n)
(0..n).each{|i|
  break if ary0[i].gcd(ary1[i]) > 1
  j = ary1[i] * ary2[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
