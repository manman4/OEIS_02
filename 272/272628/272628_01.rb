require 'prime'

ary = []
Prime.each(500000).each{|i|
  if (i * i + i + 1) % 7 == 0
    j = (i * i + i + 1) / 7
    ary << i if j.prime?
  end
}
(1..1000).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
