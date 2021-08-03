def A(k, n)
  ary = [[0]]
  a = [0]
  (1..n).each{|i|
    m = 0
    m += k ** (i / 2) if i % 2 == 1
    b = [m]
    (1..i).each{|j| b[j] = (k - 1) * a[j - 1] + b[j - 1]}
    a = b
    ary << a
  }
  ary.flatten
end

n = 139
ary = A(-2, n)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
