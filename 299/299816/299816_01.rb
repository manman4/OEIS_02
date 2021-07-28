def A(n)
  ary = [1]
  (1..n).each{|i|
    s = 0r
    s = (((i + 1) / 2) ** 2).to_r if i % 2 == 1
    (i / 2).downto(1){|j|
      s += 1
      s = j * j / s
      s += 1
      s = j * j / s
    }
    ary << 1 / (1 + s)
  }
  ary
end

n = 50
p A(n)
# A292816
p A(n).map{|i| i.numerator}
# A292817
p A(n).map{|i| i.denominator}
