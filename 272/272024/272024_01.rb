require 'prime'

def sum_of_divisors(n)
  n.prime_division.inject(1){|s, a| s *= (a[0] ** (a[1] + 1) - 1) / (a[0] - 1)}
end

n = 10000
ary = (1..n).map{|i| sum_of_divisors(i)}
N = ary.max
ps = Array.new(N + 1){0}
ps[0] = 1
(1..N).each{|num|
  (num..N).each{|i|
    ps[i] += ps[i - num]
  }
}

(1..n).each{|i| 
  print i
  print ' '
  puts ps[ary[i - 1]]
}
