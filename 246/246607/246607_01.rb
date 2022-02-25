def A(k, n)
  ary = [1] * (k + 1)
  (k + 1..n).each{|i|
    ary << ary[-1] - (k + 1) * (i - k..i - 1).inject(:*) * ary[-1 - k]
  }
  ary
end

n = 20
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if  j.to_s.size > 1000
  print i
  print ' '
  puts j
}