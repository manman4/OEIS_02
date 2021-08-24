def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A0(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) + s(30, i) + s(35, i) + s(42, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A1(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(3, i) + s(10, i) + s(14, i) + s(105, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A2(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(2, i) + s(15, i) + s(21, i) + s(70, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A3(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, i) + s(6, i) + s(7, i) + s(210, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 80
ary0 = [0] * 4 + A0(n)
ary1 = [0] * 5 + A1(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary0[i] + ary1[i]
}
