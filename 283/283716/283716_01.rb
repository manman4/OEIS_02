def s(k, i)
  s = 0
  (1..i).each{|j| s += j ** (k * j + 1) if i % j == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  n.times{|i|
    ary << (1..i + 1).inject(0){|s, j| s + ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 16
p ary = (0..n).map{|i| A(i, 3)[-1]}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

a, b, c = 3, 32, 746
a_ary = []
while a_ary.size < n + 1
  a_ary << a
  a, b, c = b, c, c * 32 - b * 139 + 108 * a
end
p a_ary == ary
