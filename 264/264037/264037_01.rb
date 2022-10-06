def A(k, n)
  a = [1]
  ary = [0]
  (1..n).each{|i|
    a << 0
    b = [0]
    (0..i - 1).each{|j|
      b[j + 1] = a[j] + (j + 1) * a[j + 1]
    }
    a = b
    # i > 0 のときa[0] = 0
    ary << (0..(i - 1) / 2).inject(0){|s, j| s + k ** j * a[2 * j + 1]}
  }
  ary
end

n = 1000
ary = A(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
