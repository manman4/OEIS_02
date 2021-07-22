def A(n)
  s = 0
  (1..n).each{|i| s += i * (2 * i - 1) if n % i == 0}
  s
end

def A278945(n)
  (0..n).map{|i| A(i)}
end

n = 10000
ary = A278945(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
