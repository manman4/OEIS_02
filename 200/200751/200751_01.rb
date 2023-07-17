# a(n) = Sum_{d|n} d*2^(d-1) 
def A(n)
  s = 0
  (1..n).each{|i| s += i * 2 ** (i - 1) if n % i == 0}
  s
end

# a(n) = -(1/n) * Sum_{m=1..n} a(n-m)*c(m) where c(m) = A083413(m)
def B(n)
  ary = [1]
  a_ary = [0] + (1..n).map{|i| A(i)}
  (1..n).each{|i|
    ary << -(1..i).inject(0){|s, j| s + a_ary[j] * ary[-j]} / i
  }
  ary
end

n = 1000
ary = B(n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}