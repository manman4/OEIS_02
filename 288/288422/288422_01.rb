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

def A(k, n)
  a_ary = [1]
  (1..n).map{|i| s(k, i)}
  a = [0] + (1..n).map{|i| s(k, i)}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

n = 34
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
