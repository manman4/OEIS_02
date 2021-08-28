# Cf. Product_{k>=1} 1/(1 - x^k)^(k^(m*k)): 
#     A000041 (m=0), 
#     A023880 (m=1), 
#     A283579 (m=2), 
#     A283580 (m=3).

def s(k, i)
  s = 0
  (1..i).each{|j| s += j ** (k * j + 1) if i % j == 0}
  s
end

def A(k, n)
  ary = [1]
  s_ary = [0] + (1..n).map{|i| s(k, i)}
  n.times{|i|
    ary << (1..i + 1).inject(0){|s, j| s + ary[-j] * s_ary[j]} / (i + 1)
  }
  ary
end

n = 12
ary = A(3, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
