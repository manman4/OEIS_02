def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
# Number of partitions of n such that (number of distinct parts) >= maximal multiplicity of the parts.
def A(n)
  cnt = 0
  partition(n, 1, n).each{|ary|
    if ary.group_by{|x| x}.size >= ary.group_by{|x| x}.values.map(&:size).max
      # p ary
      cnt += 1
    end
  }
  cnt
end
def A240306(n)
  [1] + (1..n).map{|i| A(i)}
end
p A240306(45)

