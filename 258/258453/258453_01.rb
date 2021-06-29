def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end
def A258453(n)
  ary = Array.new(n + 1, 0)
  s, t = 1, 1
  while t <= n
    a = [0] * t + I([1] + [0] * (s - 1) + [1] + [0] * (n - s), n)
    (0..n).each{|i| ary[i] += a[i]}
    s += 1
    t += s
  end
  ary
end
n = 79
ary = A258453(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
