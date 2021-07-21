require 'prime'

def A(a3, a2, a4, a6, n)
  ary = []
  Prime.take(n).each{|p|
    a = Array.new(p, 0)
    (0..p - 1).each{|i| a[(i * i + a3 * i) % p] += 1}
    ary << (0..p - 1).inject(0){|s, i| s + a[(i * i * i + a2 * i * i + a4 * i + a6) % p]}
  }
  ary
end

n = 100
ary = A(0, 0, 0, 1, n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
