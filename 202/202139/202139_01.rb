def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a[0] = 0
def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    s = (i % 2) * f(i - 1)
    (1..i / 2).each{|j|
      s += f(2 * j - 2) * ncr(i - 1, 2 * j - 1) * ary[i - 2 * j + 1]
    }
    ary << s
  }
  ary
end

n = 500
ary = A(n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}