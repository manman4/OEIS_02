require 'prime'

# m次以下を取り出す
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
  pq.each{|a, n| sum *= (power(a, (n + 1) * x) - 1) / (power(a, x) - 1)}
  sum
end

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def A(k, n)
  a = (-4 * k / bernoulli(2 * k)[-1])
  a = a.to_i if a.denominator == 1
  [1] + (1..n).map{|i| a * sigma(2 * k - 1, i)}
end

n = 30
p_ary = A(1, n)
q_ary = A(2, n)
r_ary = A(3, n)
q2_ary = mul(q_ary, q_ary, n)
p pq2_ary = mul(p_ary, q2_ary, n)
p qr_ary = mul(q_ary, r_ary, n)
(0..n).each{|i|
  j = (pq2_ary[i] - qr_ary[i]) / 720
  break if j.to_s.size > 1000 || (pq2_ary[i] - qr_ary[i]) % 720 > 0
  print i
  print ' '
  puts j
}
