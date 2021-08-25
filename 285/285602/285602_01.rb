require 'prime'

n = 300
s = 1
ary = [1] + Prime.take(n).map{|i| s *= (i * i + 1r) / (i * i - 1r)}
(0..n).each{|i|
  j = ary[i].numerator
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
