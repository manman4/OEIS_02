def r(n)
  (1..n).select{|i| n % i == 0}.size
end

def s(n)
  s = 0
  (1..n).each{|i| s += i * r(i) if n % i == 0}
  s
end

def A(n)
  a_ary = [1]
  a = [0] + (1..n).map{|i| s(i)}
  (1..n).each{|i| a_ary << (1..i).inject(0){|s, j| s - a[j] * a_ary[-j]} / i}
  a_ary
end

n = 63
ary = A(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
