require 'prime'

def A(n)
  ary = Array.new(100, 0)
  (1..n).each{|i|
    j = 2 * i
    cnt = 0
    k = 2
    while k < j
      if k.prime?
        cnt += 1 if (j - k).prime?
      end
      k += 1
    end
    ary[cnt] = i if ary[cnt] == 0
  }
  ary
end


n = 1000
p ary = A(n)
