# a(n) = 7*a(n-1) - a(n-7).
def a(n)
  ary = [1, 7, 49, 343, 2401, 16807, 117649]
  (7..n).each{|i|
    ary << 7*ary[-1] - ary[-7]
  }
  ary
end

n = 210
ary = a(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
