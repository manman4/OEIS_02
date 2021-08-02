def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A(n)
  partition(n, 1, n).map{|a| a.each_with_object(Hash.new(0)){|v, o| o[v] += 1}.values}.map{|i| i.size * i.inject(:*)}.inject(:+)
end
def A293421(n)
  (1..n).map{|i| A(i)}
end
p A293421(40)
