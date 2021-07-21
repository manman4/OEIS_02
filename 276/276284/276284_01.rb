def A(m, n)
  a = Array.new(2 * m + 1, 1)
  ary = [1]
  while ary.size < n + 1
    i = (1..m).inject(1){|s, i| s + a[2 * i - 1]} * (1..m).inject(1){|s, i| s + a[2 * i]}
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end
def A276284(n)
  A(2, n)
end

n = 25
p ary = A276284(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
