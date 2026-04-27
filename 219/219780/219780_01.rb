# a(n) = 4*n*a(n-1) - 6*(n-1)^2*a(n-2) + 4*(n-2)^3*a(n-3) - (n-3)^4*a(n-4).
def a(n)
  ary = [1, 4, 26, 220]
  (4..n).each{|i|
    ary << 4*i*ary[-1] - 6*(i-1)**2*ary[-2] + 4*(i-2)**3*ary[-3] - (i-3)**4*ary[-4]
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