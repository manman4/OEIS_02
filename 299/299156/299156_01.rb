require 'matrix'

def power(a, n, mod)
  return Matrix.I(a.row_size) if n == 0
  m = power(a, n >> 1, mod)
  m = (m * m).map{|i| i % mod}
  return m if n & 1 == 0
  (m * a).map{|i| i % mod}
end

def f(m, n, mod)
  ary0 = Array.new(m, 0)
  ary0[0] = 1
  v = Vector.elements(ary0)
  ary1 = [Array.new(m, 1)]
  (0..m - 2).each{|i|
    ary2 = Array.new(m, 0)
    ary2[i] = 1
    ary1 << ary2
  }
  a = Matrix[*ary1]
  (power(a, n, mod) * v)[m - 1]
end

def g4(n)
  m = n * (n + 1)
  m = (m * (n + 2)) / m.gcd(n + 2)
  (m * (n + 3)) / m.gcd(n + 3)
end

ary = (1..50000).select{|i| f(3, i, i * (i + 1)) == 0}
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
