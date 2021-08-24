def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k != 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 13
a = []
(1..n).each{|i| a << A(i, n - i)}
ary = []
(0..n - 1).each{|i|
  (0..i).each{|j|
    ary << a[i - j][j]
  }
}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
