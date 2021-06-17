def f(ary, n)
  return false if ary.size < n
  a = ary[-1]
  ary[-n..-2].all?{|i| i == a}
end

def A(k, n)
  f_ary = [[]]
  ary = [1]
  (n - 1).times{
    b_ary = []
    f_ary.each{|i|
      i0, i1, i2 = i + [0], i + [1], i + [2]
      b_ary << i0 if !f(i0, k)
      b_ary << i1 if !f(i1, k)
      b_ary << i2
    }
    f_ary = b_ary
    ary << f_ary.size
  }
  ary
end

n = 15
(1..10).each{|i| p A(i, n)}
