ary = [1]
s = 1
(1..10 ** 4).each{|i|
  s *= 2
  j = s.to_s.split('').map(&:to_i).select{|i| i % 2 == 1}.size
  ary << s if j == 1
}
p ary
