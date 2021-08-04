def A(k, n)
  cnt = 0
  (2..n).to_a.permutation{|a|
    flag = true
    s = 0
    t = 0
    (1..n - 2).each{|i|
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
def A291519(n)
  (1..n).each{|i| p A(3, i)}
end
A291519(10)
