def A(n)
  a = [0]
  while a.size <= n
    b = []
    a.each{|i|
      if i == 0
        b << 0
        b << 1
      elsif i == 1
        b << 0
        b << 2
      elsif i == 2
        b << 0
        b << 3
      else
        b << 0
      end
    }
    a = b
  end
  a[0..n]
end
n = 20000
ary = A(n)
(0..10000).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
