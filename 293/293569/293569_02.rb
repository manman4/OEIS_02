def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A(k, n)
  partition(n, 1, n).select{|i| i.all?{|j| j % k > 0}}.map{|a| a.each_with_object(Hash.new(0)){|v, o| o[v] += 1}.values.inject(:*)}.inject(:+)
end
def A293569(n)
  [1] + (1..n).map{|i| A(3, i)}
end
p A293569(40)
