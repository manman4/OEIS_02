require 'prime'

m = 7 * 10 ** 6
a = Prime.take(m + 7).to_a

cnt = 1
(0..m - 1).each{|i|
  if a[i] + a[i + 6] == a[i + 2] + a[i + 4] && a[i + 1] + a[i + 7] == a[i + 3] + a[i + 5]
    print cnt
    print ' '
    puts a[i]
    cnt += 1
    break if cnt > 10000
  end
}
