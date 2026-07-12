# a(n) = 6*a(n-1) - a(n-6).
def a(n)
  ary = [1, 6, 36, 216, 1296, 7776]
  (6..n).each{|i|
    ary << 6*ary[-1] - ary[-6]
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
