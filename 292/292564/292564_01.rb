def A(n)
  s = 0
  ary = []
  (1..n).each{|i|
    i.times{
      ary << s
      s += 1
    }
    s += 3 * i - 1
  }
  ary
end

n = 10000
ary = A(500)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
