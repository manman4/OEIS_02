def A(n)
  return 3 if n == 2
  cnt = 2
  while (n ** cnt - cnt ** n) % (n - cnt) > 0
    cnt += 1
  end
  cnt
end

n = 94
p (1..n).map{|i| A(i)}

