def search(a, prod, sum, size, num)
  if num == size + 1
    @cnt += 1
  else
    (1..size).each{|i|
      p, s = prod * i, sum + i
      if a[i - 1] == 0 && p % s == 0
        a[i - 1] = 1
        search(a, p, s, size, num + 1)
        a[i - 1] = 0
      end
    }
  end
end
def A(n)
  a = [0] * n
  @cnt = 0
  search(a, 1, 0, n, 1)
  @cnt
end
def A291551(n)
  (0..n).map{|i| A(i)}
end
p A291551(20)
