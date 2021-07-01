def s(f_ary, g_ary, n)
  s = 0
  (1..n).each{|i| s += i * f_ary[i] * g_ary[i] ** (n / i) if n % i == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + s(j[0], j[1], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A298906(n)
  ary1 = [0] + (1..n).map{|i| -1r / f(i)}
  ary2 = Array.new(n + 1, -1)
  A([[ary1, ary2]], n)
end

n = 500
ary = A298906(n)
(0..n).each{|i|
  j = (ary[i] * f(i)).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
