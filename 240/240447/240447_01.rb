def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
# Number of partitions of 2n such that (sum of parts having multiplicity 1) = sum of all other parts.
def A(n)
  partition(2 * n, 1, 2 * n).count{|ary|
    counts = Hash.new(0)
    ary.each{|part| counts[part] += 1}
    s1 = 0
    s2 = 0
    counts.each{|part, v|
      if v == 1
        s1 += part
      else
        s2 += part * v
      end
    }
    s1 == n && s2 == n
  }
end
n = 20
p (0..n).map{|i| A(i)}
