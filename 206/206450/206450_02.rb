# a(n) = 4*a(n-1) -a(n-4).
def a(n)
  ary = [1, 4, 16, 64]
  (4..n).each{|i|
    ary << 4*ary[-1] - ary[-4]
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
