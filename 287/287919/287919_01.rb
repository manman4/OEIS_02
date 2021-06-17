require 'prime'

def A(n)
  a = Prime.take(n).to_a
  b = [a]
  (n - 1).times{|i|
    a = (0..n - i - 2).map{|i| a[i + 1] + a[i]}
    b << a
  }
  ary = []
  (1..n).each{|i|
    (1..i).each{|j|
      ary << b[j - 1][i - j]
     }
   }
  ary
end

n = 140
ary = A(n)
(0..ary.size - 1).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
