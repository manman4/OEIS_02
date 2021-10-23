def A(n)
  s = 1
  (1..n).each{|i|
    if n % i == 0
      s *= i
    end
  }
  s
end

def B(n)
  s = 1
  (1..n).each{|i|
    if n % i == 0
      s *= A(i)
    end
  }
  s
end

n = 1200
(1..n).each{|i| 
  j = B(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
