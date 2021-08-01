def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + j[1] * s(j[0], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
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

n = 15
a = A([[5, 6], [1, -6]], n)
a2 = mul(a, a, n)
b1 = [1] + a[0..-2].map{|i| 250 * i}
(0..n - 2).each{|i|
  b1[i + 2] += 3125 * a2[i]
}
b2 = mul(b1, b1, n)
b3 = mul(b1, b2, n)
(0..n).each{|i|
  print i
  print ' '
  puts b3[i]
}
