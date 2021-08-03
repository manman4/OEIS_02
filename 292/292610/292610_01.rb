def A(n)
  s = 1
  ary = []
  (1..n).each{|i|
    (i + 2).times{
      ary << s
      s += 1
    }
    s += i
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
