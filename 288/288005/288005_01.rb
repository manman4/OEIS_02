def s(ary, n)
  a = Array.new(n, 0)
  (0..ary.size - 1).each{|i|
    (0..ary[i] - 1).each{|j|
      a[i + j] += 1
    }
  }
  a.delete(0)
  ary == a.reverse
end

def f(n, m = 0, a = [])
  if n <= m
    @ary << a.clone if n == m && a[-1] == 1 && s(a, n)
  else
    s = 1
    s = [1, a[-1] - 1].max if m > 0
    (s..n).each{|x|
      a.push x
      f(n, m + x, a)
      a.pop
    }
  end
end

n = 50
ary = []
(1..n).each{|i|
  @ary = []
  f(i)
  ary << @ary.size
p [i, ary]
}
p ary
