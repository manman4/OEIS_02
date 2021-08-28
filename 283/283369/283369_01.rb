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

n = 10
(1..n).each{|i|
  j = s(4, i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
