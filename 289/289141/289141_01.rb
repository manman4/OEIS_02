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
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A001158(n)
  s = 0
  (1..n).each{|i| s += i * i * i if n % i == 0}
  s
end

def A004009(n)
  a = [1] + (1..n).map{|i| 240 * A001158(i)}
end

def E4_3(n)
  ary = A004009(n)
  power(ary, 3, n)
end

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * a[j] * ary[-j]} / i}
  ary
end

def f(n)
  a = E4_3(n)
  b = A(-24, n)
  mul(a, b, n)
end

def A289141(n)
  f_ary0 = f(n)
  f_ary = f_ary0
  b = [[1], f_ary]
  (2..n).each{|i|
    b_ary = mul(f_ary, f_ary0, n)
    f_ary = b_ary
    b << f_ary
  }
  c = [1]
  (1..n).each{|i|
    ci = f_ary[i]
    c << - ci
    (i..n).each{|j|
      f_ary[j] -= ci * b[n - i][j - i]
    }
  }
  c.reverse
end

n = 6
a = []
(0..n).each{|i|
  j = A289141(i)
  a << j
}
ary = a.flatten!
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
