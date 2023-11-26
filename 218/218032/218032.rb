# a(0) = 1; a(n) = a(n-1) + Sum_{k=0..floor((n-3)/2)} a(k) * a(n-3-2*k).
def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << (0..(i - 3) / 2).inject(ary[-1]){|s, j| s + ary[j] * ary[i - 3 - 2 * j]}
  }
  ary
end

n = 1000
ary = [1] + A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}