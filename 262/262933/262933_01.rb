def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

def f(n)
  return 1 if n < 2
  (1..n).map{|i| 2 * i - 1}.inject(:*)
end

def B(n)
  ary0 = Array.new(n + 1, 1)
  ary1 = [0, 1, -3, 2, 2, -3, 1]
  ary2 = (0..n).map{|i| ary1[i % 7]}
  A(ary2, ary0, n)
end

n = 50
ary = B(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
