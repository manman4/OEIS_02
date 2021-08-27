require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, i)
  sum = 1
  pq = i.prime_division
  if x == 0
    pq.each{|a, n| sum *= n + 1}
  else
    pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  end
  sum
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

def p0(n)
  (3 * n * n - n) / 2
end

def p1(n)
  (3 * n * n + n) / 2
end

def A010815(n)
  ary = Array.new(n + 1, 0)
  ary[0] = 1
  i = 1
  j = p0(i)
  while j <= n
    ary[j] = (-1) ** i
    i += 1
    j = p0(i)
  end
  i = 1
  j = p1(i)
  while j <= n
    ary[j] = (-1) ** i
    i += 1
    j = p1(i)
  end
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

def A(k, l, n)
  ary = A010815(n)
  aryk = Array.new(n + 1, 0)
  (0..n / k).each{|i| aryk[k * i] = ary[i]}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  aryl = power(ps, l, n)
  mul(aryk, aryl, n)
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  ary = Array.new(n + 1, 0)
  ary[0] = 1
  ary1 = [0] + (1..n).map{|i| sigma(2, i * k) / i.to_r}
  a = ary1
  n.times{|i|
    (0..n).each{|j|
      ary[j] += a[j] / f(i + 1).to_r
    }
  a = mul(a, ary1, n)
  }
  p ary
  ary.map{|i| i.numerator}
end

n = 50
(1..3).each{|i|
  # i=3のときA283238
  p [i, A(i, n)]
}
=begin
(0..n).each{|i|
  print i
  print ' '
  puts ary[3 * i]
}
=end
