def a(n)
  a = [0, 2, 4, 6, 8] * 2
  ary = [1]
  cnt = 1
  a_cnt = 0
  s = 1
  while a_cnt < n
    if ary[0] > 4
      ary.unshift(0)
      s += 1
    end
    (0..s - 1).each{|i|
      j = ary[i]
      ary[i - 1] += 1 if j > 4
      ary[i] = a[j]
    }
    if s == ary.select{|i| i % 2 == 0}.size * 2
      a_cnt += 1
      print a_cnt
      print ' '
      puts cnt
    end
    cnt += 1
  end
end
a(1000)
