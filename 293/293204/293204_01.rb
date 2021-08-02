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

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def A(k, n)
  ary = [1]
  (1..n).each{|i|
    b_ary = Array.new(n + 1, 0)
    b_ary[0] = 1
    (1..k).each{|j|
      break if i * j > n
      b_ary[i * j] = f(j)
    }
    ary = mul(ary, b_ary, n)
  }
  ary
end

def B(k ,n)
  return [1] + Array.new(n, 0) if k == 0
  ary = A(k, n)
end

n = 500
ary = B(2, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
