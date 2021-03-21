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

def d(ary)
  s = ary.size
  return [0] if s < 2
  (1..s - 1).map{|i| i * ary[i]}
end

def A(n)
  a0, a1 = [1], [0, 0, -3]
  d = 1
  ary = [1]
  while d <= 3 * n
    s = a1.size - 1
    a20 = mul([-1, 0, 0, 8], d(a1), s + 3)
    a21 = mul([0, 0, -16 * d - 3], a1, s + 2)
    a22 = mul([0, -8 * d * d + 4 * d], a0, s + 1)
    s20, s21, s22 = a20.size - 1, a21.size - 1, a22.size - 1
    s1 = [s20, s21, s22].max
    a2 = Array.new(s1 + 1, 0)
    (0..s20).each{|i| a2[i] += a20[i]}
    (0..s21).each{|i| a2[i] += a21[i]}
    (0..s22).each{|i| a2[i] += a22[i]}
    a0, a1 = a1, a2
    d += 1
    ary << a0[0] if (d - 1) % 3 == 0
  end
  ary
end

n = 13
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
