def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = n*a(n-1) - Sum_{k=1..floor(n/2)} (-1)^k * binomial(n,2*k) * a(n-2*k).
def A(n)
  ary = [1]
  (1..n).each{|i|
    s = i * ary[i - 1]
    (1..i / 2).each{|k|
      s -= (-1) ** k * ncr(i, 2 * k) * ary[i - 2 * k]
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
