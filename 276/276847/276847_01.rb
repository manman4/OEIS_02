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

def A(n)
  ary = A010815(n)
  ary2 = Array.new(n + 1, 0)
  (0..n / 2).each{|i| ary2[2 * i] = ary[i]}
  ary4 = Array.new(n + 1, 0)
  (0..n / 4).each{|i| ary4[4 * i] = ary[i]}
  ary6 = Array.new(n + 1, 0)
  (0..n / 6).each{|i| ary6[6 * i] = ary[i]}
  ary12 = Array.new(n + 1, 0)
  (0..n / 12).each{|i| ary12[12 * i] = ary[i]}
  ary0204 = mul(ary2, ary4, n)
  ary0612 = mul(ary6, ary12, n)
  mul(ary0204, ary0612, n)[0..-2]
end

n = 100
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
