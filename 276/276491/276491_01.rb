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
def A010815(n)
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    ary = mul(ary, b_ary, n)
  }
  ary
end
def A276491(n)
  ary = A010815(n)
  ary2 = Array.new(n + 1, 0)
  (0..n / 2).each{|i| ary2[2 * i] = ary[i]}
  ary10 = Array.new(n + 1, 0)
  (0..n / 10).each{|i| ary10[10 * i] = ary[i]}
  b = mul(ary2, ary10, n)
  mul(b, b, n)[0..-2]
end

n = 100
p ary = A276491(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
