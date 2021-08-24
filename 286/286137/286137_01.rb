def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) + s(2, i) + s(15, i) + s(30, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 78
ary = [0] * 2 + A(n)[0..-3]
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
