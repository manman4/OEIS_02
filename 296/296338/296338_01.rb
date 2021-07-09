def A(n)
  m = Math.sqrt(n).to_i
  ary = Array.new(n + 1, 0)
  (1..m).each{|i|
    sum = i * i
    ary[sum] += 1
    i += 1
    sum += i * i
    while sum <= n
      ary[sum] += 1
      i += 1
      sum += i * i
    end
  }
  ary
end

n = 10000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
