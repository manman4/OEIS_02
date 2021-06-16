def f(n)
  s = 0
  0.upto(n){|x|
    0.upto(n){|y|
      if  x * x + y * y <= n
        s += 1
      end
    }
  }
  s
end

n = 1000
(0..n).each{|i|
  print i
  print ' '
  puts f(i)
}
