def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A286128(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) + s(12, i) + s(15, i) + s(20, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  ary
end

n = 79
ary = [0, 0] + A286128(n)[0..-3]
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
