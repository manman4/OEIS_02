def s(k, m, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == m}
  s
end

def A(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - a[j] * ary[-j]} / i}
  a_ary = Array.new(n + 1, 0)
  (0..n / k).each{|i| a_ary[k * i] = ary[i]}
  a_ary
end

def B(k, n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(5, 1, i) + s(5, 4, i) - s(5, 2, i) - s(5, 3, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - k * a[j] * ary[-j]} / i}
  ary
end

n = 56
a = A(2, n)
b = B(2, n)
(0..n).each{|i|
  j = a[i] - b[i]
  break if j % 2 > 0
  print i
  print ' '
  puts j / 2
}
