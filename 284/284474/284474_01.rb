def s(k, m, n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i * i if n % i == 0 && i % k == m}
  s
end
def A083365(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(2, 1, i) - s(2, 0, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - ary[-j] * s_ary[j]} / i}
  ary
end

n = 36
p ary = A083365(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}

def B(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(2, 1, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - ary[-j] * s_ary[j]} / i}
  ary
end
def C(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| - s(2, 0, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - ary[-j] * s_ary[j]} / i}
  ary
end

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

b = B(n)
c = C(n)
(0..n / 2).map{|i| c[i * 2]}
p mul(b, c, n)
