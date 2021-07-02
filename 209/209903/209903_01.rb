def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def s(n)
  s = 0
  (1..n).each{|i| s += 1r / f(i) if n % i == 0}
  n * s
end

def A(n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s + a[j] * a_ary[-j]} / i}
  a_ary
end

n = 500
ary = A(n)
(0..n).each{|i|
  j = (ary[i] * f(i)).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
