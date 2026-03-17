def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
# A(n, k) is the number of partitions of n such that the sum of parts with multiplicity 1 is k, and the sum of parts with multiplicity greater than 1 is n-k.
def A(n, k)
  partition(n, 1, n).count{|ary|
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
    s1 == k && s2 == n - k
  }
end
n = 20
(0..10).each{|i| p (0..i).map{|j| A(i, j)}}
p (0..n).map{|i| A(2 * i, i)}
