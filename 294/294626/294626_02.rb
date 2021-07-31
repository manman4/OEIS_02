require 'prime'

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

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

# x > 0
def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  sum
end

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
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

def E_2k(k, n)
  a = (-4 * k / bernoulli(2 * k)[-1])
  b = a.denominator
  c = a.numerator
  [b] + (1..n).map{|i| c * sigma(2 * k - 1, i)}
end

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def A294626(n)
  a1 = I(E_2k(1, n), n)
  b2 = E_2k(2, n)
  c12 = mul(a1, b2, n)
  a2 = I(E_2k(2, n), n)
  b3 = E_2k(3, n)
  c23 = mul(a2, b3, n)
  m_ary = [0] + (1..n).map{|i| A008683(i)}
  ary = []
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += m_ary[i / j] * (c12[j] - c23[j]) if i % j == 0
    }
    ary << s / (24 * i)
  }
  ary
end

n = 13
ary = A294626(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
