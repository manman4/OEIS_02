require 'prime'
cnt = 1
Prime.each.take(10000).each{|i|
  print cnt
  print ' '
  puts i * (i - 1) * (501 * i ** 3 - 414 * i * i + 111 * i - 54) / 120
  cnt += 1
}
