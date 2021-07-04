require 'prime'

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

def A(n)
  ary = [1]
  (1..n).each{|i|
    if i.prime?
      b_ary = Array.new(i + 1, 0)
      b_ary[0], b_ary[-1] = 1, -1
      ary = mul(ary, b_ary, n)
    end
  }
  mul(ary, ary, n)
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

n = 50
ary = A(n)
ary = I(ary, n)
(0..n).each{|i|
  print i
  print ' '
  puts ary[i]
}
