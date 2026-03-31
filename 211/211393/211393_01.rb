def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

# a(0) = 1; a(n) = Sum_{k=1..n} binomial(2*n-1,2*k-1) * (2*(k-1))! * a(n-k).
def A(n)
  # 階乗の配列を作る
  f_ary = [1]
  (1..2 * n).each{|i| f_ary << f_ary[-1] * i}
  ary = [1]
  (1..n).each{|i| ary << (1..i).inject(0){|s, k| s + ncr(2 * i - 1, 2 * k - 1) * f_ary[2 * (k - 1)] * ary[-k]}}
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