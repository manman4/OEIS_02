def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** (k * i + 1) if n % i == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i|
    ary << -(1..i).inject(0){|s, j| s + s_ary[j] * ary[-j]} / i
  }
  ary
end

def A283675(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (0..n).each{|i|
    (0..i).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n =8
ary = A283675(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
