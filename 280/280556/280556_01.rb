def A280556(n)
  (0..n).map{|i| (i - 1) * (1..i + 2).inject(:*) + 2}
end

n = 500
ary = A280556(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
