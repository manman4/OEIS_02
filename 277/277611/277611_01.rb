def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| i ** (i - 2)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]}}
  ary
end

n = 400
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}