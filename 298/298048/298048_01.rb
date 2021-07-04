require 'prime'

def A(n)
  ary = [2, 3, 5, 7]
  p [ary.size, ary]
  (n - 1).times{|i|
    ary1 = []
    ary.each{|a|
      (1..9).each{|d|
        j = d * 10 ** (i + 1) + a
        ary1 << j if j.prime?
        j = a * 10 + d
        ary1 << j if j.prime?
      }
    }
    ary = ary1.uniq
    p [ary.size, ary]
  }
end

A(5)
