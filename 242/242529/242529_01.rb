# A086595(n=1..36)のデータはb086595.txtに保存されている。
# 形式は
# n a(n)
# n<3 の時は、1
# For n>2, a(n) = A086595(n)/2.

n = 36
a086595 = File.read("b086595.txt").split("\n").map{|l| l.split.map(&:to_i)}.to_h
(1..n).each{|i|
  print i
  print ' '
  if i < 3
    puts "1"
  else
    puts a086595[i]/2
  end
}

