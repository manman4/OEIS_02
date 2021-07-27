require 'prime'

def bernoulli(n)
  ary = []
  a = []
  (0..n).each{|i|
    a << 1r / (i + 1)
    i.downto(1){|j| a[j - 1] = j * (a[j - 1] - a[j])}
    ary << a[0] # Bn = a[0]
  }
  ary
end

def A(n)
  a = bernoulli(2 * n)
  ary = []
  (1..n).each{|i|
    j = (- a[2 * i] / (2r * i)).numerator.abs
    if (17..19).include?(i)
      ary << j
    elsif j > 1
      ary << j.prime_division[-1][0]
    else
      ary << j
    end
  }
  ary
end

n = 19
p ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
