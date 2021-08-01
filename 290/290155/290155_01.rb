n = 64
@ary = [0, 10]
(2..n).each{|i|
  str = i.to_s(2)
  @ary << ('1' + @ary[str.size - 1].to_s + str[1..-1]).to_i
}
p @ary
(0..n).each{|i|
  print i
  print ' '
  puts @ary[i]
}
