# a(n) = 3*n*a(n-1) - 3*(n-1)^2*a(n-2) + (n-2)^3*a(n-3)
def a(n)
  ary = [1, 3, 15]
  (3..n).each{|i|
    ary << 3*i*ary[-1] - 3*(i-1)**2*ary[-2] + (i-2)**3*ary[-3]
  }
  ary
end

n = 1000
ary = a(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}