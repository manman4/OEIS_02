def A(n, k)
  cnt = 0
  a = (1..n).to_a
  # derangement のうち、pi(i) > i となる i がちょうど k 個の順列の数を加算
  cnt += a.permutation.count{|perm|
    perm.each_with_index.none?{|v, i| v == i + 1} &&
    perm.each_with_index.count{|v, i| v > i + 1} == k
  }
  cnt
end
n = 8
ary = []
(0..n).each{|i|
  (0..i).each{|j|
    ary << A(i, j)
  }
}
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
