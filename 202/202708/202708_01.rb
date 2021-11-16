def A(n)
  n.to_s.split('').inject(0){|s, i| s + i.to_i}
end

def B(n)
  ary = [1]
  s = 1
  (1..n).each{|i|
    s *= i
    ary << A(s)
  }
  ary
end

n = 75
ary = B(n)
(6..n).each{|i|
  print i
  print ' '
  puts ary[i] / 9
}
