require 'prime'

ary = []
Prime.each(1100000).each{|i|
  if (i * i + i + 1) % 13 == 0
    j = (i * i + i + 1) / 13
    ary << i if j.prime?
  end
}
(1..1000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
