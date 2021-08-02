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

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
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
  b_ary = [0] + (1..n).map{|i| ncr(i + k - 1, k)}
  c_ary = (0..n + k + 1).map{|i| 1r / f(i)}
  x_ary = [0, 1]
  x_ary = S(b_ary, x_ary, n)
  x_ary = S(c_ary, x_ary, n)
  (0..n).map{|i| (f(i) * x_ary[i]).to_i}
end

def A293012(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = [1]
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
    ary << 1
  }
  ary
end
n = 139
ary = A293012(n - 1)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
