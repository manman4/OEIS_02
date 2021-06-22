require 'prime'

def A275742(n)
  ary = []
  cnt = 1
  Prime.each(2 * 10 ** 7){|p|
    s = 0
    (0..p - 1).each{|i|
      (0..p - 1).each{|j|
        if (j * j + i * j + j - i * i * i -  i * i + 10 * i + 10) % p == 0
          s += 1
        end
      }
    }
    ary << s
    cnt += 1
    return ary if cnt > n
  }
end

n = 1000
ary = A275742(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
