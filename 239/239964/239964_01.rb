def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
# Number of partitions of n such that (number of distinct parts) = maximal multiplicity of the parts.
def A(n)
  cnt = 0
  partition(n, 1, n).each{|ary|
    if ary.group_by{|x| x}.size == ary.group_by{|x| x}.values.map(&:size).max
      # p ary
      cnt += 1
    end
  }
  cnt
end
def A239964(n)
  [0] + (1..n).map{|i| A(i)}
end
n = 50
ary = A239964(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}

