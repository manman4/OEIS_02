def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, i) + s(6, i) + s(7, i) + s(210, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 81
ary = [0] * 9 + A(n)[0..-10]
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
