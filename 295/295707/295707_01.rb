def f(m, n, k)
  s = 0
  t, u = n / k, m / k
  (-t).upto(t){|x|
    (-u).upto(u){|y|
      if x.gcd(y) == 1
        s += (n - (k * x).abs) * (m - (k * y).abs)
      end
    }
  }
  s
end

def g(m, n)
  return 0 if m == 0 || n == 0
  (f(m, n, 1) - f(m, n, 2)) / 2
end

def A(m, n)
  (0..n).map{|i| g(m, i)}
end

def B(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (1..n).each{|i|
    (1..i - 1).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 141
ary = B(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
