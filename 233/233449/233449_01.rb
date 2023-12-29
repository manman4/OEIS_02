def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

# a(0) = 1; a(n) = 2*a(n-1) + n!.
def A(n)
  ary = [1]
  (1..n).each{|i|
    ary << 2 * ary[-1] + f(i)
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
