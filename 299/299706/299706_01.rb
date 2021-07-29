def f(a, b, c, n)
  return 0 if a + b + c > n
  s = n / (a + b + c)
  s += f( a - 2 * b + 2 * c,  2 * a - b + 2 * c,  2 * a - 2 * b + 3 * c, n)
  s += f( a + 2 * b + 2 * c,  2 * a + b + 2 * c,  2 * a + 2 * b + 3 * c, n)
  s += f(-a + 2 * b + 2 * c, -2 * a + b + 2 * c, -2 * a + 2 * b + 3 * c, n)
  return s
end
def A299706(n)
  (1..n).map{|i| f(3, 4, 5, 10 ** i)}
end
p A299706(8)

n = 500
p ary = [0] + (1..n).map{|i| f(3, 4, 5, i)}
p (1..n).map{|i| ary[i] - ary[i - 1]}
