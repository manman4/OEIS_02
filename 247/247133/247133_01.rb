# mŸˆÈ‰º‚ğæ‚èo‚·
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

def A247133(n)
  ary = A010815(n)
  ary12 = Array.new(n + 1, 0)
  (0..n / 12).each{|i| ary12[12 * i] = ary[i]}
  ary11 = [1]
  11.step(n, 12){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary11 = mul(ary11, b_ary, n)
  }
  ary1 = [1]
  1.step(n, 12){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary1 = mul(ary1, b_ary, n)
  }
  ary12 = mul(ary12, ary11, n)
  mul(ary12, ary1, n)
end

n = 1000
ary = A247133(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
