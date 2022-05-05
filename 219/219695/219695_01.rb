require 'prime'

def A(n)
  return [1, n] if n.prime?
  (1..n).select{|i| n % i == 0}
end

def B(n)
  ary = A(2 * n - 1)
  m = ary.size
  if m % 2 == 1
    return 0
  else
    return (ary[m / 2] - ary[m / 2 - 1]) / 2
  end
end

n = 10000
(1..n).each{|i|
  j = B(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}