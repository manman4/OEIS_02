def f(n)
  return 1 if n < 2
  return (1..n).inject(:*)
end

def A(n)
  a = [1]
  ary = [1]
  (1..n).each{|i|
    a << 0
    b = [0]
    (0..i - 1).each{|j|
      b[j + 1] = a[j] + (j + 1) * a[j + 1]
    }
    a = b
    m = f(i)
    ary << (0..i).inject(0){|s, j| s + j ** i * m * a[j] / f(j)}
  }
  ary
end

n = 300
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
