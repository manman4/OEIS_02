def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a002893(n) = Sum_{k=0..n} binomial(n,k)^2 * binomial(2*k,k).
def a002893(n)
  (0..n).inject(0){|s, k| s + ncr(n, k)**2 * ncr(2*k, k)}
end

# a(n) = Sum_{k=0..n} binomial(n,k) * binomial(n+k,k) * A002893(k).
def a(n)
  ary = [1]
  a_ary = (0..n).map{|i| a002893(i)}
  (1..n).each{|i|
    ary << (0..i).inject(0){|s, k| s + ncr(i, k) * ncr(i+k, k) * a_ary[k]}
  }
  ary
end

n = 630
ary = a(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}