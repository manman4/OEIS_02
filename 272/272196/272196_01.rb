require 'prime'

def A272196(n)
  ary = []
  Prime.take(n).each{|p|
    a = Array.new(p, 0)
    (0..p - 1).each{|i| a[(i * i) % p] += 1}
    ary << (0..p - 1).inject(0){|s, i| s + a[(i * i * i - 4 * i * i + 16) % p]}
  }
  ary
end

n = 100
ary = A272196(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
