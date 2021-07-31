def f(m, n)
  s = 0
  (-m).upto(m){|x|
    (-n).upto(n){|y|
      if x.gcd(y) == 1
        s += 1
      end
    }
  }
  s
end

def A(m, n)
  (0..n).map{|i| f(m, i)}
end

n = 100
p ary = A(3, n)
(1..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
