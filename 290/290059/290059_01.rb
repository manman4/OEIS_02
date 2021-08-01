require 'prime'

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

n = 300
p_ary = Prime.take(n).to_a
(1..n).each{|i|
  j = ncr(2 * p_ary[i - 1] - 1, p_ary[i - 1] - 1)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
