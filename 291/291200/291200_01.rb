def A(k, n)
  ary = []
  1.step(2 * k + 2, 2){|i| ary << i}
  ps = Array.new(n + 1){0}
  ps[0] = 1
  ary.each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

def B(n)
  ary = Array.new(n + 1, 0)
  (0..n).each{|i|
    a = [0] * (i * (i + 1)) + A(i, n)
    (0..n).each{|j| ary[j] += a[j]}
  }
  ary
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

n = 50
ary = I(B(n), n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
