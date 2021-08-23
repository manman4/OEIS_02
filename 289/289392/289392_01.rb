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

def E_2k(k, n)
  a = (-4 * k / bernoulli(2 * k)[-1])
  b = a.denominator
  c = a.numerator
  [b] + (1..n).map{|i| c * sigma(2 * k - 1, i)}
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

def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end

def A288968(n)
  a = I(E_2k(1, n), n)
  b = E_2k(2, n)
  c = mul(a, b, n)
  m_ary = [0] + (1..n).map{|i| A008683(i)}
  ary = []
  (1..n).each{|i|
    s = 0
    (1..i).each{|j|
      s += m_ary[i / j] * c[j] if i % j == 0
    }
    ary << 2 + s / (12 * i)
  }
  ary
end

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

n = 16
a = A288968(n)
d = [0] + a.map{|i| i / 4}
p d.select{|i| i.denominator != 1}

ary = [1]
(1..n).each{|i|
  x = [1] + [0] * (i - 1) + [-1]
  y = power(x, d[i], n)
  ary = mul(ary, y, n)
}
ary
(1..4).each{|i|
  # i=1のときA289392
p [i, power(ary, i, n)]
}

x =
[-240,26760,-4096240,708938760,-130880766192,
 25168873498760,-4978357936128240,
 1005225129317834760,-206195878414962688240,
 42824436296045618358408,
 -8983966738037593190400240,
 1900416270294787067711818760,
 -404814256845771786255876096240,
 86744167089111545378556727322760]
p x.map{|i| i / 8}
y =
[504,143388,51180024,20556578700,8806299845112,
 3929750661380124,1803727445909594616,
 845145871847732769804,402283166289266872824312,
 193877350835487271784566812,
 94381548697864188120110027256,
 46328820782943001597184984563596]
p y.map{|i| i / 12}
