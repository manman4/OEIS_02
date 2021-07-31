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

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

def A(k, n)
  ary = [1] + Array.new(n, 0)
  (1..k).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, 1
    ary = mul(ary, b_ary, n)
  }
  I(ary, n)
end

def B(k, n)
  p_ary = A(k, n)
  ary = [1]
  (1..n).each{|i|
    ary << (f(i - 1) * (1..i).inject(0){|s, j| s + j * p_ary[j] * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end

n = 20
ary = B(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
