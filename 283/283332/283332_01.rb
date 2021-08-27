def A(k, n)
  a = Array.new(2 * k + 1, 1)
  ary = [1]
  while ary.size < n
    j = (1..k).inject(0){|s, i| s + a[- i * 2 + 1] * a[- i * 2]}
    break if j % a[0] > 0
    a = *a[1..-1], j / a[0]
    ary << a[0]
  end
  ary
end

n = 20
p ary = A(4, n)
(1..n).each{|i|
   j = ary[i - 1]
   break if j.to_s.size > 1000
   print i
   print ' '
   puts j
}
