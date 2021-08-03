def A(n)
  s = 4
  ary = []
  (1..n).each{|i|
    i.times{
      ary << s
      s += 1
    }
    s += i + 3
  }
  ary
end

n = 10000
ary = A(150).map{|i| i * (i + 1) / 2}
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
