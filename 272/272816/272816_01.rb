require 'prime'

cnt = 1
Prime.each(10 ** 7).each{|i|
  if (i + 20).prime?
    print cnt
    print ' '
    puts i
    cnt += 1
    print cnt
    print ' '
    puts i + 20
    cnt += 1
  end
  break if cnt > 10000
}
