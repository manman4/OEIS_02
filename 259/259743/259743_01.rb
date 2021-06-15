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

def A010816(n)
  ary = Array.new(n + 1, 0)
  # ヤコビの公式の必要なところだけ取り出す
  i = 0
  j, k = 2 * i + 1, i * (i + 1) / 2
  while k <= n
    i & 1 == 1? ary[k] = -j : ary[k] = j
    i += 1
    j, k = 2 * i + 1, i * (i + 1) / 2
  end
  ary
end

def A259743(n)
  ary = A010815(n)
  ary8 = Array.new(n + 1, 0)
  (0..n / 8).each{|i| ary8[8 * i] = ary[i]}
  ary1 = A010816(n)
  ary4 = [1]
  4.step(n, 4){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, 1
    ary4 = mul(ary4, b_ary, n)
  }
  ary = mul(ary1, ary4, n)
  mul(ary, ary8, n)
end

n = 1000
ary = A259743(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
