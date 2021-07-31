require 'prime'

def power(a, n)
  return 1 if n == 0
  k = power(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

# Divisor Function
def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power(a, (n + 1) * x) - 1) / (power(a, x) - 1)}
  sum
end

def A(k, n)
  ary = [1]
  s_ary = []
  a = [0] + (1..n).map{|i| sigma(k, i)}
  (1..n).each{|i|
    j = (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i
    ary << j
    
    if j % i == 0
      s_ary << i
      p i
    end
  }
  s_ary
end

n = 10 ** 4
p ary = A(2, n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
