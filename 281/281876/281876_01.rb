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

# m���ȉ������o��
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

def s(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0}
  s
end

def A000594(n)
  ary = [1]
  a = [0] + (1..n - 1).map{|i| s(i)}
  (1..n - 1).each{|i| ary << (1..i).inject(0){|s, j| s - 24 * a[j] * ary[-j]} / i}
  ary
end

n = 20
q_ary = A(2, n)
t_ary = [0] + A000594(n)
ary = mul(q_ary, t_ary, n)
(1..n).each{|i|
  j = (sigma(15, i) - ary[i]) / 3617
  break if j.to_s.size > 1000 || (sigma(15, i) - ary[i]) % 3617 > 0
  print i
  print ' '
  puts j
}
