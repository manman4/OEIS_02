def A(n)
  ary = [1]
  a = [2, -1]
  i = 0
  while i < n
    ary << a
    a = [2] + (0..i).map{|i| a[i] + a[i + 1]} + [-1]
    i += 1
  end
  ary.flatten
end

n = 11
ary = A(n)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
