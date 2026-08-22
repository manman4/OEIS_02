require 'prime'

def A(n)
  m = Math.sqrt(n).to_i + 1
  ary = Array.new(n + 1, 0)
  (1..m).each{|i|
    (1..m).each{|j|
      k = i * i + 2 * j * j
      if k <= n
        ary[k] += 1
      end
    }
  }
  ary
end

n = 7 * 10 ** 7
ary = A(n)

p a = (0..100).map{|i| ary.index(i)}

