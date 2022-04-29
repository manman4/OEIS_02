# Cf. A193375

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
      k = j * j
      break if k > i
      s += k * ary[i - k] * m / f(i - k)
    }
    ary << s
  }
  ary
end

n = 22
ary = A(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}