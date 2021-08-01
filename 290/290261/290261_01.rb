def A(n)
  ary = [0, 1]
  (2..n).each{|i|
    s = (2 ** i - 1r) / i
    (2..i).each{|j|
      s += (-ary[i / j]) ** j / j.to_r if i % j == 0
    }
    ary << s
  }
  ary
end

n = 5000
ary = A(n)
(1..n).each{|i|
  j = ary[i].to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
