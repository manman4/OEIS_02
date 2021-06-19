def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def d(ary)
  (1..ary.size - 1).map{|i| i * ary[i]}
end

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(n)
  ary = [1]
  (0..n - 1).each{|i|
    a = [0] * (i + 1) + (0..i).map{|j| ncr(i, j)}
    i.times{ a = d(a)}
    m = f(i)
    ary << (0..i + 1).map{|j| a[j] / m}
  }
  ary.flatten
end

n = 139
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
