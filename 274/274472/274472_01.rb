require 'prime'

def a(n)
  return 0 if n.prime?
  return a(3 * n + 1) + 1 if n % 2 == 1
  a(n / 2) + 1
end

n = 10000
(1..n).each{|i|
  print i
  print ' '
  puts a(i)
}
