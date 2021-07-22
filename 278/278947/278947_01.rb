def A(k, n)
  s = 0
  (1..n).each{|i| s += k * i * (i - 1) / 2 - i * (i - 2) if n % i == 0}
  s
end

def A278947(n)
  (0..n).map{|i| A(7, i)}
end

n = 10000
ary = A278947(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
