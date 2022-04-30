def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  ary = [1]
  (1..n).each{|i|
    m = f(i - 1)
    s = 0
    (1..i).each{|j|
      s += j / ((j + k - 1) / k).to_r * ary[i - j] * m / f(i - j)
    }
    ary << s.to_i
  }
  ary
end

n = 500
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}