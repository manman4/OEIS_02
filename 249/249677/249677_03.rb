# T(n,k) = Sum_{1 <= x_1 < x_2 < ... < x_k <= n} ( Product_{j=1..k} x_j )^3.
def A(n, k)
  return 1 if k == 0
  return 0 if n < k
  sum = 0
  (1..n).to_a.combination(k){|a|
    s = a.inject(1){|r, i| r * i}
    sum += s ** 3
  }
  sum
end

n = 10
p ary = (0..n).map{|i| (0..i).map{|j| A(i, j)}}.flatten
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
