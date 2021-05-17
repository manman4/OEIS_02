def A(n)
  m = Math.sqrt(n).to_i
  ary = Array.new(n + 1, 0)
  (1..m).each{|i|
   
    sum = i * i
    ary[sum] += 1
    i += 1
    
    sum += i * i
    while sum <= n
      j = ary[sum]
      ary[sum] += 1
      i += 1
      
      sum += i * i
    end
  }
  ary
end

a = A(2 * 10 ** 8)
p (1..10).map{|i| a.index(i)}
p (1..2 * 10 ** 5).select{|i| a[i] == 2}
p (1..2 * 10 ** 5).select{|i| a[i] == 3}
