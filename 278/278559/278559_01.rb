def A000041(n)
  ps = Array.new(n + 1){0}
  ps[0] = 1
  (1..n).each{|num|
    (num..n).each{|i|
      ps[i] += ps[i - num]
    }
  }
  ps
end

n = 1000
a = A000041(25 * n + 24)
ary = (0..n).map{|i| a[25 * i + 24]}
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
