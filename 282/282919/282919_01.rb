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

# mŸˆÈ‰º‚ğæ‚èo‚·
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A(k, l, n)
  aryk = power(A010815(n), k, n)
  aryk7 = Array.new(n + 1, 0)
  (0..n / 7).each{|i| aryk7[7 * i] = aryk[i]}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  aryl = power(ps, l, n)
  mul(aryk7, aryl, n)
end

n = 30
p ary4 = A(4, 5, n)
p ary8 = [0] * 1 + A(8, 9, n)
p ary12 = [0] * 2 + A(12, 13, n)
p ary16 = [0] * 3 + A(16, 17, n)
p ary20 = [0] * 4 + A(20, 21, n)
p ary24 = [0] * 5 + A(24, 25, n)
p ary28 = [0] * 6 + A(28, 29, n)
p ary32 = [0] * 7 + A(32, 33, n)
p ary36 = [0] * 8 + A(36, 37, n)
p ary40 = [0] * 9 + A(40, 41, n)
p ary44 = [0] * 10 + A(44, 45, n)
p ary48 = [0] * 11 + A(48, 49, n)
p ary52 = [0] * 12 + A(52, 53, n)
p ary56 = [0] * 13 + A(56, 57, n)
p ary = (0..n).map{|i|
  2546 * 7**2 * ary4[i] + 48934 * 7**4 * ary8[i] + 1418989 * 7**5 * ary12[i] + 2488800 * 7**7 * ary16[i] + 2394438 * 7**9 * ary20[i] + 1437047 * 7**11 * ary24[i] + 4043313 * 7**12 * ary28[i] + 161744 * 7**15 * ary32[i] + 32136 * 7**17 * ary36[i] + 31734 * 7**18 * ary40[i] + 3120 * 7**20 * ary44[i] + 204 * 7**22 * ary48[i] + 8 * 7**24 * ary52[i] + 7**25 * ary56[i]
}

  ps = Array.new(49*n+47 + 1){0}
  ps[0] = 1
  (1..49*n+47).each{|num|
    (num..49*n+47).each{|i|
      ps[i] += ps[i - num]
    }
  }
p (0..n).map{|i| ps[49*i+47]} == ary
=begin
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
=end
