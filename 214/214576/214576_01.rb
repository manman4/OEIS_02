def partition(n, min, max)
  return [[]] if n == 0
  [max, n].min.downto(min).flat_map{|i| partition(n - i, min, i).map{|rest| [i, *rest]}}
end

def f(n)
  a = Array.new(n, 0)
  partition(n, 1, n).each{|ary|
    flag = true
    (0..ary.size - 2).each{|i|
      if ary[i] % ary[i + 1] > 0
        flag = false
        break
      end
    }
    a[ary[-1] - 1] += 1 if flag
  }
  a
end

def A(n)
  (1..n).map{|i| f(i)}.flatten
end

n = 13
p ary = A(n)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
