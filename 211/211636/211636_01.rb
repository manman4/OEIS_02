# Sum_{k=1..n-1} floor(sqrt(n^2-k^2)).
def A(n)
  s = 0
  (1..n - 1).each{|i|
    s += Math.sqrt(n * n - i * i).to_i
  }
  s
end

n = 10000
s = 0
(0..n).each{|i|
  s += A(i)
  break if s.to_s.size > 1000
  print i
  print ' '
  puts s
}
