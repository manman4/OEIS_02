def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end
  
def A(n)
  m = f(n)
  (1..n).each{|i|
    return i if m <= i ** n
  }
end

n = 1000
(1..n).each{|i|
  print i
  print ' '
  puts A(i)
}