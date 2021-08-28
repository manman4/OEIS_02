def s(k, i)
  s = 0
  (1..i).each{|j| s += j ** (k * j + 1) if i % j == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - ary[-j] * s_ary[j]} / i}
  ary
end

def A283720(n)
  (0..n).map{|i| A(i, i)[-1]}
end

n = 8
ary = A283720(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
