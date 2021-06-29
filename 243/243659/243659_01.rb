def ncr(n, r)
  return 0 if r < 0
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def A(n)
  a = [1]
  (1..n).each{|i|
    a << (1..i).inject(0){|s, j|
      s + (0..3*i).inject(0){|t, k|
        # k-2*j<0のときもあり
        t + 2**(k-2*j) * (-1)**(j-k) * ncr(j,3*j-k) * ncr(j+k-1,j-1)
      } * a[i-j]
    } / i
  }
  a
end

n = 21
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j.to_i
}
