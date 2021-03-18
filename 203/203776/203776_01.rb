# m次以下を取り出す
def mul(f_ary, b_ary, m)
  s1, s2 = f_ary.size, b_ary.size
  ary = Array.new(s1 + s2 - 1, 0)
  (0..s1 - 1).each{|i|
    (0..s2 - 1).each{|j|
      ary[i + j] += f_ary[i] * b_ary[j]
    }
  }
  ary[0..m]
end

# A133100 を参考にした。
def A203776(n)
  ary4 = [1]
  4.step(n, 5){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, 1
    ary4 = mul(ary4, b_ary, n)
  }
  ary1 = [1]
  1.step(n, 5){|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, 1
    ary1 = mul(ary1, b_ary, n)
  }
  mul(ary4, ary1, n)
end

n = 1000
ary = A203776(n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
