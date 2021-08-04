def search(a, sum, k, size, num)
  if num == size + 1
    @cnt += 1
  else
    (1..size).each{|i|
      if a[i - 1] == 0 && (sum + i ** k) % num == 0
        a[i - 1] = 1
        search(a, sum + i ** k, k, size, num + 1)
        a[i - 1] = 0
      end
    }
  end
end
def A(k, n)
  a = [0] * n
  @cnt = 0
  search(a, 0, k, n, 1)
  @cnt
end
def A291355(n)
  (0..n).map{|i| A(3, i)}
end
p A291355(10)
