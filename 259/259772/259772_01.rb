require 'prime'

n = 1000
p_ary = Prime.each(10 ** 7).to_a
a = 0
cnt = 2
p, q, r = 2, 3, 5
while a < n
  if (p * p * p + q * q + r).prime?
    a += 1
    print a
    print ' '
    puts p
  end
  cnt += 1
  p, q, r = q, r, p_ary[cnt]
end
