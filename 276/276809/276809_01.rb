require 'prime'

def A276809(n)
  p_ary = Prime.each(10 ** 7).to_a
  ary = []
  (0..n - 1).each{|i|
    j = p_ary[i]
    p_ary.each{|k|
      m = k ** 2 - 1
      if m % 24 == 0
        m /= 24
        if m % j == 0
          ary << k
          break
        end
      end
    }
  }
  ary
end

n = 100
ary = A276809(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
