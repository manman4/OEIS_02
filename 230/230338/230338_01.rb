def A003487(n)
  ary = [5]
  n.times{ary << ary[-1] ** 2 - 2}
  ary
end

def A230338(n)
  ary = [1]
  return ary if n == 0
  a = A003487(n - 1)
  s = 1
  n.times{|i|
    s *= a[i]
    ary << s
  }
  ary
end

n = 15
ary = A230338(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
