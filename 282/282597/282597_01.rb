require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, i)
  return i ** x if i < 2
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
  sum
end

def A(k, n)
  (0..n).map{|i| i * sigma(k, i)}
end

n = 10100
ary = A(13, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
