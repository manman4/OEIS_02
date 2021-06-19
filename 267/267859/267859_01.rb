require 'prime'
def A267859(n)
  ary = []
  cnt = 1
  Prime.take(10 ** 7).each{|p|
    a = Array.new(p, 0)
    (0..p - 1).each{|i| a[(i * i) % p] += 1}
    s = 0
    (0..p - 1).each{|i|
      s += a[(i * i * i - 4 * i) % p]
    }
    if p - s != 0
      ary << p - s
      cnt += 1
      return ary if cnt > n
    end
  }
end

n = 100
ary = A267859(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
