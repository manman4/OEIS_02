require 'prime'

def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, i)
  sum = 1
  pq = i.prime_division
  if x == 0
    pq.each{|a, n| sum *= n + 1}
  else
    pq.each{|a, n| sum *= (power(a, (n + 1) * x) - 1) / (power(a, x) - 1)}
  end
  sum
end

def A(k, m, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| sigma(k, i * m)}
  n.times{|i|
    ary << -(1..i + 1).inject(0){|s, j| s + ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 11
a = []
(1..n).each{|i| a << A(i, 1, n - i)}
p a
ary = []
(1..n).each{|i|
  (1..i).each{|j|
    ary << a[i - j][j - 1]
  }
}
p ary
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
