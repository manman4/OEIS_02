def A220849(n)
  s = 1
  (1..n).each{|i|
    if n % i == 0
      (1..i).each{|j|
        if n % j == 0
          s *= j
        end
      }
    end
  }
  s
end

n = 2000
(1..n).each{|i| 
  j = A220849(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
