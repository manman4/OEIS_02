require 'prime'

def A(k, n)
  ary = [2, 3, 5, 7]
  p [ary.size, ary]
  (n - 1).times{|i|
    ary1 = []
    ary.each{|a|
      if i % 2 == k
        (1..9).each{|d|
          j = (d.to_s + a.to_s).to_i
          ary1 << j if j.prime?
        }
      else
        # 1の桁が0の素数はない
        (1..9).each{|d|
          j = (a.to_s + d.to_s).to_i
          ary1 << j if j.prime?
        }
      end
    }
    ary = ary1.uniq
    p [ary.size, ary]
  }
end

# 最初左から加える
A(0, 16)
p ''
# 最初右から加える
A(1, 14)
