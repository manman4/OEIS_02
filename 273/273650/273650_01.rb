# m���ȉ������o��
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  s10 = [s1 - 1, m].min
  (0..s10).each{|i|
    s20 = [s2 - 1, m - i].min
    (0..s20).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary
end

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A000594(n)
  ary = Array.new(n + 1, 0)
  # ���R�r�̌����̕K�v�ȂƂ��낾�����o��
  i = 0
  j, k = 2 * i + 1, i * (i + 1) / 2
  while k <= n
    i & 1 == 1? ary[k] = -j : ary[k] = j
    i += 1
    j, k = 2 * i + 1, i * (i + 1) / 2
  end
  # 8�悵��x�{
  power(ary, 8, n).unshift(0)[1..n]
end

n = 82
ary = A000594(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1] % i
}
