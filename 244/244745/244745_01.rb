def s(k, n)
  s = 0
  (1..n).each{|i| s += i if n % i == 0 && i % k == 0}
  s
end

def A(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(5, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s - 6 * a[j] * ary[-j]} / i}
  ary
end

def B(n)
  ary = [1]
  a = [0] + (1..n).map{|i| s(1, i) - s(5, i)}
  (1..n).each{|i| ary << (1..i).inject(0){|s, j| s + 6 * a[j] * ary[-j]} / i}
  ary
end

n = 25
a_ary = A(n + 1)
b_ary = [0, 0] + B(n - 1)
-1.upto(n){|i|
  print i
  print ' '
  puts a_ary[i + 1] + 125 * b_ary[i + 1]
}
