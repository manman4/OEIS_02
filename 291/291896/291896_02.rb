def f(n, m = 1, a = [1])
  if n <= m
    @ary << a.clone if n == m
  else
    al = a[-1]
    ([1, al - 1].max..al + 1).each{|x|
      a.push x
      f(n, m + x, a)
      a.pop
    }
  end
end

n = 20
ary = [1]
(1..n).each{|i|
  @ary = []
  f(i)
  ary << @ary.size
}
p ary
ary.flatten!
p ary
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
