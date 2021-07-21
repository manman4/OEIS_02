def A(k, n)
  a = Array.new(2 * k + 1, 1)
  ary = [1]
  while ary.size < n + 1
    i = 0
    k.downto(1){|j|
      i += 1
      i *= a[j] * a[-j]
    }
    break if i % a[0] > 0
    a = *a[1..-1], i / a[0]
    ary << a[0]
  end
  ary
end

def A276534(n)
  A(2, n)
end

n = 20
ary = A276534(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
