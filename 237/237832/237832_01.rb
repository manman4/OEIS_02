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

def f(n)
  ary = [0] * (n + 1)
  # Sum_{k>=1} (-1)^(k-1) * k * ( x^(k*(3*k-1)/2) + x^(k*(3*k+1)/2) )
  s = 1
  while
    t = s * (3 * s - 1) / 2
    break if t > n
    ary[t] += (-1) ** (s - 1) * s
    s += 1
  end
  s = 1
  while t = s * (3 * s + 1) / 2
    break if t > n
    ary[t] += (-1) ** (s - 1) * s
    s += 1
  end
  ary
end

def f1(n)
  ary = [0] * (n + 1)
  # Sum_{k>=1} (-1)^(k-1) * x^(3*k*(k-1)/2) * (1-x^(2*k))
  s = 1
  while t = 3 * s * (s - 1) / 2
    break if t > n
    ary[t] += (-1) ** (s - 1)
    s += 1
  end
  s = 1
  while t = 3 * s * (s - 1) / 2 + 2 * s
    break if t > n
    ary[t] -= (-1) ** (s - 1)
    s += 1
  end
  ary
end

def s(f_ary, g_ary, n, m)
  s = 0
  (m..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(ary, n, m = 1)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + s(j[0], j[1], i, m)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

def g(n)
  ary1 = Array.new(n + 1, 1)
  # Product_{k>0} 1/(1 - x^k)
  A([[ary1, ary1]], n)
end

def g1(n)
  ary1 = [1, 2] + Array.new(n - 1, 1)
  ary2 = Array.new(n + 1, 1)
  # Product_{k>0} 1/(1 - x^k) * (x/(1 - x))
  [0] + A([[ary1, ary2]], n)[0..-2]
end

def h(n)
  a = mul(f(n), g(n), n)
  b = mul(f1(n), g1(n), n)
  (0..n).map{|i| a[i] - b[i]}
end

n = 54
m = 54
p ary = h(n)
(1..m).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
