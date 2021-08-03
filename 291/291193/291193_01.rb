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

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

n = 50
a = [
1, -1, 2, -2, 2, -3, 4, -4, 5, -6, 6, -8, 10, -10, 12, -14, 15, -18, 20, -22, 26, -29, 32, -36, 40, -44, 50, -56, 60, -68, 76, -82, 92, -101, 110, -122, 134, -146, 160, -176, 191, -210, 230, -248, 272, -296, 320, -350, 380, -410, 446, -484, 522, -566, 612, -660, 715, -772, 830, -896, 966, -1038
]
p ary = I(a, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
