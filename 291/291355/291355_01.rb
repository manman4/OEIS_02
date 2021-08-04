def A(k, n)
  cnt = 0
  (1..n).to_a.permutation{|a|
    flag = true
    s = 0
    (1..n).each{|i|
      s += a[i - 1] ** k
      if s % i > 0
        flag = false
        break
      end
    }
    cnt += 1 if flag
  }
  cnt
end
def A291355(n)
  (1..n).map{|i| A(3, i)}
end
p A291355(8)
