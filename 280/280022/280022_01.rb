require 'prime'

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, i)
  sum = 1
  pq = i.prime_division
  pq.each{|a, n| sum *= (power0(a, (n + 1) * x) - 1) / (power0(a, x) - 1)}
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
  b = a.denominator
  c = a.numerator
  [b] + (1..n).map{|i| c * sigma(2 * k - 1, i)}
end

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

# m次以下を取り出す
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

n = 1100
p_ary = A(1, n)
p2_ary = power(p_ary, 2, n)
p3_ary = power(p_ary, 3, n)
p5_ary = power(p_ary, 5, n)
q_ary = A(2, n)
q2_ary = power(q_ary, 2, n)
r_ary = A(3, n)
pq2_ary = mul(p_ary, q2_ary, n)
p2r_ary = mul(p2_ary, r_ary, n)
p3q_ary = mul(p3_ary, q_ary, n)
qr_ary = mul(q_ary, r_ary, n)
(0..n).each{|i|
  j = 15 * pq2_ary[i] - 20 * p2r_ary[i] + 10 * p3q_ary[i] - 4 * qr_ary[i] - p5_ary[i]
  break if j % 20736 > 0
  print i
  print ' '
  puts j / 20736
}
