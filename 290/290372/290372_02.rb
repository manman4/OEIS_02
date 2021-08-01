def A290372(n)
  m = 10 ** (n + 1)
  str = (m + (2 ** 5 ** n - 5 ** 2 ** n) % m).to_s.reverse
  (0..n).map{|i| str[i].to_i}
end


n = 10
p ary = A290372(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
