def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
def A293847(n)
  ary = [1]
  (1..n).each{|i|
    ary << (f(i - 1) * (1..i).inject(0){|s, j| s + j * f(j) * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end
n = 17
ary = A293847(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
