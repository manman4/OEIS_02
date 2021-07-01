def A(n)
  return 0 if n < 2
  (1..n).inject(:*) - 1
end

def B(n)
  a = [1]
  s_ary = [[1]]
  ary = [1, 1]
  i = 1
  while ary.size < n
    b = a * (i + 1)
    c = ary[0..A(i)] + s_ary[1..-1].flatten
    (0..c.size - 1).each{|j| b[j] += c[j]}
    a = b
    s_ary.unshift(a)
    ary += a * (i + 1)
    i += 1
  end
  ary[0..n]
end

n = 100000
a = B(n)
ary = (0..n).select{|i| a[i] == 1}
(1..10000).each{|i|
  print i
  print ' '
  puts ary[i]
}
