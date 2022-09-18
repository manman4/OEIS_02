def A(x, y)
  m = y / x
  m += 1 if y % x > 0
  m
end

def B(n, a = [])
  return [1] if n == 1
  x, y = n.numerator, n.denominator
  return a if y == 1
  m = A(x, y)
  a << m
  x, y = (-y) % x, y * m
  n = x.to_r / y
  return B(n, a)
end

def C(n)
  ary = []
  (1..n).each{|i|
    ary << B(i.to_r / (i + 1))
  }
  ary.flatten
end

n= 1000
ary = C(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}