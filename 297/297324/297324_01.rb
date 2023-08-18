def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(f_ary, g_ary, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(f_ary, g_ary, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + s_ary[j] * ary[-j]} / i}
  ary
end

def B(n)
  ary0 = (0..n).map{|i| -n}
  ary1 = [0] + (1..n).map{|i| i}
  A(ary0, ary1, n)[-1]
end

n = 1000
(0..n).each{|i|
  j = B(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
