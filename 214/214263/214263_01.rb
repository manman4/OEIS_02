def A214263(n)
  ary = Array.new(n + 1, 0)
  i = 0
  while true
    j = 4 * i * i - 3 * i
    break if j > n
    ary[j] = 1
    i += 1
  end
  i = 1
  while true
    j = 4 * i * i + 3 * i
    break if j > n
    ary[j] = 1
    i += 1
  end
  ary
end

n = 10000
ary = A214263(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
