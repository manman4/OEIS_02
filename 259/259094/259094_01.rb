def T(n, k)
  ary = []
  1.step(2 * k, 2){|i| ary << i}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps[n]
end

def A(k, n)
  (0..n).map{|i| T(i, k)}
end

n = 140
a = []
(0..n).each{|i| a << A(i, n - i)}
ary = []
(0..n).each{|i|
  (0..i - 1).each{|j|
    ary << a[i - j][j]
  }
}
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
