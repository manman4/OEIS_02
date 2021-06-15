def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def f(n)
  return 1 if n == 0
  cnt = 0
  partition(n, 1, n).each{|ary|
    ary0 = (1..ary.size - 1).map{|i| ary[i - 1] - ary[i]}
    cnt += 1 if ary0.sort == ary0.reverse
  }
  cnt
end
def A240026(n)
  (0..n).map{|i| f(i)}
end
p A240026(50) 
