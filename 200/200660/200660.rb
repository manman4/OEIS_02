def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def ncr(n, r)
  return 1 if r == 0
  (n - r + 1..n).inject(:*) / (1..r).inject(:*)
end

def stirling_2(n, k)
  (0..k).inject(0){|s, i| s + (-1) ** ((k - i) % 2) * i ** n * ncr(k, i)} / f(k)
end

def A(n)
  s_ary = (0..n).map{|i| stirling_2(n, i)}
  (1..n).inject(0){|s, i| s + s_ary[i] * i * (n - i)}
end

n = 500
(1..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
