require 'prime'
cnt = 1
Prime.each.take(10000).each{|i|
  print cnt
  print ' '
  puts i * (i - 1) * (4051 * i ** 4 - 4130 * i ** 3 + 1445 * i * i - 190 * i + 264) / 720
  cnt += 1
}
