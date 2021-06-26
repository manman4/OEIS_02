require 'prime'

def A(n)
  p_ary = Prime.each(10 ** 7).to_a
  ary = []
  i = 1 # p_ary[1] = 3
  while ary.size < n
    j = p_ary[i] ** 2
    ary << j if ((j + 1) / 2).prime?
    i += 1
  end
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
