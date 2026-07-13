# a(n) = 8*a(n-1) - a(n-8).
def a(n)
  ary = [1, 8, 64, 512, 4096, 32768, 262144, 2097152]
  (8..n).each{|i|
    ary << 8*ary[-1] - ary[-8]
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
