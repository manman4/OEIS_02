# a(0) = 1; a(n) = Sum_{j=0..floor((n-1)/2)} a(n-1-2*j) * Sum_{k=0..j} a(k)*a(j-k).

def A(n)
  ary = [1]
  (1..n).each{|i| ary << (0..(i-1)/2).inject(0){|s, j| s + ary[i-1-2*j] * (0..j).inject(0){|t, k| t + ary[k]*ary[j-k]}}}
  ary
end

n = 1000
ary = A(n)
(0..n).each{|i| 
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print " "
  puts j
}
