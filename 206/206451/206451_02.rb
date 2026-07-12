# a(n) = 5*a(n-1) -a(n-5).
def a(n)
  ary = [1, 5, 25, 125, 625]
  (5..n).each{|i|
    ary << 5*ary[-1] - ary[-5]
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
