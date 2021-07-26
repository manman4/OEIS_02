require 'prime'

N = 2 * 10 ** 8
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

ary = [0, 0, 0, 0, 0] + (5..10 ** 8).map{|i| A(i)}
p (5..100).map{|i| ary.find_index(i)}
