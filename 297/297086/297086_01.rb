def phi(n)
  (1..n).count{|i| i.gcd(n) == 1}
end

def A(n)
  return 1 if n.gcd(phi(n)) == 1
  0
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}
