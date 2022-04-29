def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = [1]
  (1..n).each{|i|
    m = f(i - 1)
    s = 0
    (1..i).each{|j|
      break if j * j > i
      s += ary[i - j * j] * m / f(i - j * j)
    }
    ary << s
  }
  ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}