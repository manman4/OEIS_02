require 'prime'
cnt = 1
Prime.each.take(10000).each{|i|
  print cnt
  print ' '
  puts i * (i - 1) * (73 * i * i - 45 * i + 14) / 24
  cnt += 1
}
