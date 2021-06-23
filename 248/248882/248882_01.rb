def s(k, n)
  s = 0
  (1..n).each{|i| s += (-1) ** (n / i + 1) * i ** k if n % i == 0}
  s
end

def A(k, n)
  ary = [1]
  p a = [0] + (1..n).map{|i| s(k + 1, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + a[j] * ary[-j]} / i}
  ary
end

n = 27
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
