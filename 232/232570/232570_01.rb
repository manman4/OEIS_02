require 'matrix'

def power(a, n, mod)
  return Matrix.I(a.row_size) if n == 0
  m = power(a, n >> 1, mod)
  m = (m * m).map{|i| i % mod}
  return m if n & 1 == 0
  (m * a).map{|i| i % mod}
end

def f(m, n)
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
  mod = n
  (power(a, n, mod) * v)[m - 1]
end

cnt = 1
(1..10 ** 9).each{|i|
  j = f(3, i)
  if j == 0
    print cnt
    print ' '
    puts i
    cnt += 1
  end
  break if cnt > 10000
}
