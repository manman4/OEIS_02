def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(ary, n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| ary.inject(0){|s, j| s + j[1] * s(j[0], i)}}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

def B(k, n)
  A([[2 * k, 1], [k, -1], [2, -1], [1, 1]], n)
end

n = 56
ary = B(6, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
