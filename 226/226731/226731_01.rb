def A226731(n)
  (3..n).map{|i| (1..2 * i - 1).inject(:*) / (2 * i)}
end

n = 300
ary = A226731(n)
(3..n).each{|i|
  j = ary[i - 3]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
