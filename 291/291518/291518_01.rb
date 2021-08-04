def A(k, m, n)
  cnt = 0
  (1..n).to_a.permutation{|a|
    next if a[n - 1] != m
    flag = true
    s = 0
    t = 0
    (1..n).each{|i|
      s += a[i - 1] ** k
      t += a[i - 1]
      if s % t > 0
        flag = false
        break
      end
    }
    cnt += 1 if flag
  }
  cnt
end
def A291518(n)
  (1..n).each{|i| p A(3, i, i)}
end
def A291519(n)
  (1..n).each{|i| p A(3, 1, i)}
end
A291518(9)
A291519(9)
