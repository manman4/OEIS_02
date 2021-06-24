require 'prime'
cnt = 1
Prime.each.take(10000).each{|i|
  print cnt
  print ' '
  puts i * (i - 1) * (13 * i - 5) / 6
  cnt += 1
}
