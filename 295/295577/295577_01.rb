def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = [0, 1]
  (1..n - 1).each{|i|
    s = 0
    (1..i).each{|j| s += ary[j] / (f(j) * f(i / j).to_r) if i % j == 0}
    ary << (f(i) * s).to_i
  }
  ary
end

n = 26
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
