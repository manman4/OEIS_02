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

def f(n)
  return 1 if n < 2
  (1..n).inject(:*)
end

def power0(a, n)
  return 1 if n == 0
  k = power0(a, n >> 1)
  k *= k
  return k if n & 1 == 0
  return k * a
end

def sigma(x, mod, i)
  sum = 0
  (1..i).each{|j|
    sum += j ** x if i % j == 0 && j % mod > 0
  }
  sum
end

def A(k, n)
  p_ary = [0] + (1..n).map{|i| sigma(k, 2, i)}
  ary = [1]
  (1..n).each{|i|
    ary << (-1) * (f(i - 1) * (1..i).inject(0){|s, j| s + j * p_ary[j] * ary[i - j] / f(i - j).to_r}).to_i
  }
  ary
end

n = 50
ary = A(0, n)
(0..n).each{|i|
  j = ary[i]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
