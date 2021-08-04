def search(a, prod, sum, size, num)
  if num == size + 1
    @ary << a.clone
  else
    (1..size).each{|i|
      p, s = prod * i, sum + i
      if a[i - 1] == 0 && p % s == 0
        a[i - 1] = num
        search(a, p, s, size, num + 1)
        a[i - 1] = 0
      end
    }
  end
end

def A(n)
  a = [0] * n
  @ary = []
  search(a, 1, 0, n, 1)
  @ary.map{|i| (1..n).map{|j| i.index(j) + 1}}
end

n = 16
(1..n).each{|i| p A(i).size}
