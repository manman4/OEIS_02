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

# 1����n
def A006571(n)
  ary = A010815(n)
  ary11 = Array.new(n + 1, 0)
  (0..n / 11).each{|i| ary11[11 * i] = ary[i]}
  b = mul(ary, ary11, n)
  mul(b, b, n)[0..-2]
end

def A275578(n)
  ary = A006571(2 * n + 1)
  (0..n).map{|i| ary[2 * i]}
end

n = 11000
ary = A275578(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
