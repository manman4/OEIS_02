def A(n)
  s = 0
  ary = []
  (1..n).each{|i|
    s += 3 * i - 2
    (i - 1).times{
      ary << s
      s += 1
    }
  }
  ary
end

p ary = A(15)
(1..56).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
