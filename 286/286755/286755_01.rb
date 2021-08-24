def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end
def A(n)
  ary = []
  (1..n).each{|m|
    partition(m, 1, m).each{|a|
      s = a.size
      if s == a.uniq.size
        ary << (-1) ** s
      else
        ary << 0
      end
    }
  }
  ary
end

def A080577(n)
  ary = []
  (1..n).map{|m| partition(m, 1, m)}
end

ary = A(9)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
