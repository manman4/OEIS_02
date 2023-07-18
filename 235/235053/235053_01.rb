require 'prime'

def A(n)
  ary = []
  cnt = 1
  while ary.size < n
    if cnt.prime?
      m = cnt * cnt + 1
      ary << m if (m * m + 1).prime?
    end
    cnt += 1
  end
  ary
end

n = 1000
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
