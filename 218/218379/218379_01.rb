n = 10 ** 4
ary = (1..n).map{|i| i * (3 * i - 1) / 2}
ps = Array.new(n + 1){0}
ps[0] = 1
ary.each{|num|
  (num..n).each{|i|
    ps[i] += ps[i - num]
  }
}

(0..n).each{|i|
  print i
  print ' '
  puts ps[i]
}
