def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << (1..i).inject(0){|s, j| s + f(3 * j) / f(j) ** 3 * ary[-j]} / i
  }
  ary
end

n = 800
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}