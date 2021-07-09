def d(n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % 2 == 1}
  s
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

n = 21
ary0 = (0..n).map{|i| (2 * i + 1) ** 2}
p ary = I(ary0, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
