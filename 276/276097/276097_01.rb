def A(m, n)
  a = Array.new(m, 1)
  ary = [1]
  while ary.size < n
    i = a[1..-1].inject(:+)
    j = i * i
    break if j % a[0] > 0
    a = *a[1..-1], j / a[0]
    ary << a[0]
  end
  ary
end

def A276097(n)
  A(5, n)
end

n = 18
ary = A276097(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}