def A(m, n)
  a = Array.new(m, 1)
  ary = [1]
  while ary.size < n
    a = *a[1..-1], *a[1..-1].inject(:*) * (m + 1) - a[0]
    ary << a[0]
  end
  ary
end

def A276258(n)
  A(3, n)
end

n = 20
ary = A276258(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
