# m”{‚µ‚½‚ç
def A(m, n)
  str = n.to_s
  (m * n).to_s == str[-1] + str[0..-2]
end

m = 6
n = 5
(1..n).each{|i|
  (1..9).each{|j|
    p [i, j] if A(m, j * (10 ** (58 * i) - 1) / 59)
  }
}
n = 2
ary = []
(1..n).each{|i|
  (6..9).each{|j|
    ary << j * (10 ** (58 * i) - 1) / 59
  }
}
p ary
