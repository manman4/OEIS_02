def A274032(n)
  ary = [3]
  i = 0
  a, b, c = 3, -9, 83
  while i < n
    a, b, c = b, c, -9 * c + b + a
    ary << a
    i += 1
  end
  ary
end

n = 1000
ary = A274032(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
