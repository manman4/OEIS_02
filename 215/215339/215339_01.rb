def A001608(n)
  a, b, c = 3, 0, 2
  ary = [3]
  n.times{
    a, b, c = b, c, a + b
    ary << a
  }
  ary
end

n = 10000
ary = A001608(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i] % i
}
