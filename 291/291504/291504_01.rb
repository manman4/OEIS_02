require 'prime'
def A(k, n)
  ary = []
  (1..n).to_a.permutation{|a|
    flag = true
    s = 0
    (1..n).each{|i|
      s += a[i - 1] ** k
      if s.prime?
        flag = false
        break
      end
    }
    ary << a if flag
  }
  ary
end
def A291504(n)
  (0..n).map{|i| A(1, i).size}
end
p A291504(7)
