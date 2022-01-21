def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A(k, n)
  cnt = 0
  partition(n, 1, n).each{|ary|
    cnt += 1 if k * ary[0] == ary.size
  }
  cnt
end

n = 100
(1..n).each{|i|
  print i
  print ' '
  puts A(2, i)
}
