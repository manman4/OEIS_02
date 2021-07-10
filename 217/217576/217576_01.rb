def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(i)
  s = 0
  (1..i).each{|j| s += f(j) ** (i / j) if i % j == 0}
  s
end

n = 500
ary = (1..n).map{|i| A(i)}
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
