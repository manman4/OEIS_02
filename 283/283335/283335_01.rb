def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def s(i)
  s = 0
  (1..i).each{|j| s += j ** j if i % j == 0}
  s
end

def A283335(n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(i)}
  n.times{|i|
    ary << (1..i + 1).inject(0){|s, j| s - ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 18
ary = A283335(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
