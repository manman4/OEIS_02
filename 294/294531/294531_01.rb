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

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def I(ary, n)
  a = [1]
  (0..n - 1).each{|i| a << -(0..i).inject(0){|s, j| s + ary[1 + i - j] * a[j]}}
  a
end

# n���܂�
def q(n)
  return [1] if n == 0
  ary = [1]
  (1..n).each{|i|
    if i % 2 > 0
      b_ary = Array.new(i + 1, 0)
      b_ary[0], b_ary[-1] = 1, -1r / f(i)
      ary = mul(ary, b_ary, n)
    end
  }
  I(ary, n)
end

n = 50
ary = q(n)
b=[]
(0..n).each{|i|
  j = (f(i) * ary[i]).to_i
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
b<<j
}
p b
