def s(k, n)
  s = 0
  (1..n).each{|i| s += n / i if n % i == 0 && i % k > 0}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + k * a[j] * ary[-j]} / i}
  ary
end

n = 100
ary = A(4, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
