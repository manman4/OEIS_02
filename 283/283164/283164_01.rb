# m次以下を取り出す
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

# m次以下を取り出す
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A283164(n)
  ary = A010815(n)
  ary112 = power(ary, 12, n)
  ary6 = Array.new(n + 1, 0)
  (0..n / 6).each{|i| ary6[6 * i] = ary[i]}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ary2 = Array.new(n + 1, 0)
  (0..n / 2).each{|i| ary2[2 * i] = ps[i]}
  ary3 = Array.new(n + 1, 0)
  (0..n / 3).each{|i| ary3[3 * i] = ps[i]}
  ary24 = power(ary2, 4, n)
  ary33 = power(ary3, 3, n)
  a = mul(ary112, ary6, n)
  b = mul(ary24, ary33, n)
  mul(a, b, n)
end

n = 1100
ary = A283164(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
