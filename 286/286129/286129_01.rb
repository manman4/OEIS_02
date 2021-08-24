def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A286129(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(3, i) + s(4, i) + s(5, i) + s(60, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 78
ary = [0, 0, 0] + A286129(n)[0..-4]
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
