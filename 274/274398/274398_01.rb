require 'prime'

def f(n)
  i = 1
  flag = true
  while flag
    i += 1
    j = n - i * i
    flag = false if Math.sqrt(j).to_i ** 2 == j
  end
  j = Math.sqrt(n - i * i).to_i
  i, j = j, i if i % 2 == 0
  return i if (i - j - 1) % 4 == 0
  -i
end

def A274398(n)
  ary = []
  p_ary = Prime.each(4 * n + 1).select{|i| i % 4 == 1}
  4.step(4 * n, 4){|i|
    j = 1r / 2
    p_ary.each{|k|
      break if k - 1 > i
      j += (2r * f(k)) ** (i / (k - 1)) / k if i % (k - 1) == 0
    }
    ary << j.numerator
  }
  ary
end

n = 4000
ary = A274398(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
