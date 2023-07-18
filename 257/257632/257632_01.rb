def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(3, i) + s(4, i) + s(5, i) + s(60, i) - s(2, i) - s(6, i) - s(10, i) - s(30, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

def A132968(n)
  [1] + A(n)[0..-2].map{|i| -i}
end

n = 10000
ary = A132968(2 * n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[2 * i]
}
