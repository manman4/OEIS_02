def A(n)
  s = 1
  f = [[1]]
  ary = [1]
  1.upto(n){|i|
    s = 0
    a = []
    ((Math.sqrt(i * 8 + 1) - 1) / 2).floor.downto(1){|j|
      a.unshift(s += f[i - j][j - 1])
    }
    f.push(a.unshift(s))
    ary << s
  }
  ary
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

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

# m���ȉ������o��
def power1(ary, n, m)
  if n == 0
    a = Array.new(m + 1, 0)
    a[0] = 1
    return a
  end
  k = power1(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

n = 11
p ary0 = I(A(n), n)
a = []
(0..n).each{|i| a << power1(ary0, i, n - i)}
p a
ary = []
(0..n).each{|i|
  (0..i).each{|j|
    ary << a[i - j][j]
  }
}
p ary
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
