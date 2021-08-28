def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def s(k, i)
  s = 0
  (1..i).each{|j| s += j ** (k * j + 1) if i % j == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  n.times{|i|
    ary << (1..i + 1).inject(0){|s, j| s - ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 12
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
