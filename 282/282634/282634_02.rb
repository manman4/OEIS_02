require 'prime'
def A008683(n)
  ary = n.prime_division
  return (-1) ** (ary.size % 2) if ary.all?{|i| i[1] == 1}
  0
end
def a(n, t)
  m = n.gcd(t)
  s = 0
  (1..m).each{|i|
    s += i * A008683(n / i) if m % i == 0
  }
  s
end
def A282634(n)
  (1..n).map{|i| (0..i - 1).map{|j| a(i, j)}}.flatten
end
def a0(n, t)
  m = n.gcd(n - t)
  s = 0
  (1..m).each{|i|
    s += i * A008683(n / i) if m % i == 0
  }
  s
end
def A282634_0(n)
  (1..n).map{|i| (0..i - 1).map{|j| a(i, j)}}.flatten
end
n = 35
p ary = A282634(n)
p ary == A282634_0(n)
