require 'prime'

M = 10 ** 7
N = 2 * 5 * M
@ary = Array.new(N + 1, 0)
Prime.each(N){|i|
  @ary[i] = 1
}
(1..N).each{|i|
  @ary[i] += @ary[i - 1]
}

def A(n)
  (1..N / n).each{|i|
    return i if @ary[i * n] == i + n
  }
  nil
end

p (5..M).select{|i| A(i) == 5}
