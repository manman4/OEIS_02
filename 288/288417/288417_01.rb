def s0(n)
  s = 0
  1.step(n, 2){|i| s += i if n % i == 0}
  s
end

def s(k, n)
  s = 0
  (1..n).each{|i| s += i ** k * s0(n / i) if n % i == 0}
  s
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts s(0, i)
}
