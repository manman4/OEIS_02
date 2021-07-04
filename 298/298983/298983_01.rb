# m���ȉ������o��
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

# m���ȉ������o��
def power(ary, n, m)
  return [1] if n == 0
  k = power(ary, n >> 1, m)
  k = mul(k, k, m)
  return k if n & 1 == 0
  return mul(k, ary, m)
end

def A(k, n)
  s_ary = [[1]]
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(i + 1, 0)
    b_ary[0], b_ary[-1] = 1, -1
    b_ary = power(b_ary, k, k * i)
    ary = mul(ary, b_ary, k * i * (i + 1) / 2)
    ary0 = []
    (0..i).each{|j|
      s = 0
      j.step(ary.size - 1, i + 1){|k|
        s += ary[k]
      }
      ary0 << s
    }
    s_ary << ary0
  }
  s_ary
end

n = 10
ary = A(2, n).flatten
(0..ary.size - 1).each{|i|
  print i
  print ' '
  puts ary[i]
}
