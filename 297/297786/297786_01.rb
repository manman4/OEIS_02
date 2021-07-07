require 'bigdecimal'

n = 10000
ary = BigDecimal(10980011.to_r / 999 ** 3, n + 1000).to_s("F").split('').map(&:to_i)
ary.shift
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
