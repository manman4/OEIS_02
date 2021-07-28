def A001158(n)
  s = 0
  (1..n).each{|i| s += i * i * i if n % i == 0}
  s
end

def A004009(n)
  a = [1] + (1..n).map{|i| 240 * A001158(i)}
end

def E4(n)
  ary = A004009(n)
end

# ary[0] = 1, ary[1] �͋���
def sqrt_a(ary)
  n = ary.size - 1
  a = [1, ary[1] / 2]
  i = 1
  while i < n
    a << (ary[i + 1] - (1..i).inject(0){|s, j| s + a[j] * a[-j]}) / 2
    i += 1
  end
  a
end

def A108091(n)
  a0 = E4(n)
  a1 = sqrt_a(a0)
  a2 = sqrt_a(a1)
  sqrt_a(a2)
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

def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
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

def A106205(n)
  a = A108091(n)
  b = A(-1, n)
  mul(a, b, n)
end

def I(ary, n)
  a = [1]
  i = 0
  while i < n
    a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}
    i += 1
  end
  a
end

n = 12
ary = I(A106205(n), n)
# (1..24).each{|i|
#   p [i, power(ary, i, n)]
# }
ary = power(ary, 2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
