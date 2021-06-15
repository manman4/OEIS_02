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

def A(k, n)
  ary = []
  x = [1]
  a = [1] * (k + 1)
  (n + 1).times{|i|
    ary << x
    x = mul(x, a, k * (i + 1))
  }
  ary.flatten
end

n = 46
ary = A(9, n)
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}