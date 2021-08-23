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

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

def A(k, n)
  ary = [1] + Array.new(n, 0)
  i = 1
  while i <= n
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -i
    ary = mul(ary, b_ary, n)
    i *= k
  end
  I(ary, n)
end

n = 30
ary = A(2, 2 * n)
(0..n).each{|i|
  j = ary[2 * i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
