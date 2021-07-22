require 'prime'

def phi(n)
  a = n.prime_division.to_a
  a.inject(n){|s, i| s * (1 - 1r / i[0])}.to_i
end

def A278049(n)
  ary = []
  s = 0
  (1..n).each{|i|
    s += phi(i)
    ary << 3 * s - 1
  }
  ary
end

n = 100
p ary = A278049(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
