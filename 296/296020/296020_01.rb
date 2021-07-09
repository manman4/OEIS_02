require 'prime'
def A(k, n)
  ary = []
  cnt = 0
  k.step(4 * n + k, 4){|i|
    cnt += 1 if i.prime?
    ary << cnt
  }
  ary
end

n = 10000
ary = A(3, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
