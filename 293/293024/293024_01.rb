# m���ȉ������o��
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def S(c_ary, x_ary, n)
  ary = Array.new(n + 1, 0)
  tq = x_ary
  tqn = [1] + Array.new(n, 0)
  (0..c_ary.size - 1).each{|i|
    (0..n).each{|j|
      ary[j] += c_ary[i] * tqn[j]
    }
    tqn = mul(tqn, tq, n)
  }
  ary
end

def A(k, n)
  c_ary = (0..n + k + 1).map{|i| 1r / f(i)}
  d_ary = c_ary.clone
  (0..k).each{|i| d_ary[i] = 0}
  x_ary = [0, 1]
  x_ary = S(d_ary, x_ary, n)
  x_ary = S(c_ary, x_ary, n)
  (0..n).map{|i| (f(i) * x_ary[i]).to_i}
end

def A293024(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end
n = 11
p ary = A293024(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
