def a(n)
  str = (2 ** n).to_s
  str.size - str.split('').map(&:to_i).select{|i| i % 2 == 0}.size * 2
end
(0..78).each{|i| p a(i)}
