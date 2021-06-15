require 'prime'

n = 10000
s = 0
t = 0
Prime.take(n).map{|i|
  s += (-1) ** (t % 2) * i * i
  t += 1
  print t
  print ' '
  puts s
}
