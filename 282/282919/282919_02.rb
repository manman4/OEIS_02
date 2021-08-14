def A(n)
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

n = 500
ary = A(49 * n + 47)
(0..n).each{|i|
  print i
  print ' '
  puts ary[49 * i + 47]
}
