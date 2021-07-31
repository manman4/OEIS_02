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

def B(n)
  a = []
  (0..n).each{|i| a << A(i, n - i)}
  ary = []
  (1..n).each{|i|
    (1..i - 1).each{|j|
      ary << a[i - j][j]
    }
  }
  ary
end

n = 141
ary = B(n)
(1..ary.size).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
