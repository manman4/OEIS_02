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

# mŸˆÈ‰º‚ğæ‚èo‚·
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A(k, n)
  ary = Array.new(2 * k + 1, 1)
  power(ary, n, k * n)[-1]
end

n = 500
(0..n).each{|i|
  j = A(5, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
