# m”{‚µ‚½‚ç
def A(m, n)
  str = n.to_s
  (m * n).to_s == str[-1] + str[0..-2]
end

m = 9
n = 5
(1..n).each{|i|
  (1..9).each{|j|
    p [i, j] if A(m, j * (10 ** (44 * i) - 1) / 89)
  }
}
n = 3
ary = []
(1..n).each{|i|
  (9..9).each{|j|
    ary << j * (10 ** (44 * i) - 1) / 89
  }
}
p ary
