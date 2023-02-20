require 'prime'

def sum_digit(n)
  n.to_s.split('').map(&:to_i).inject(:+)
end

ary = Prime.each(10000000).to_a
s = ary.size
cnt = 1
(0..s - 2).each{|i|
  if ary[i + 1] - ary[i] == 18
    if sum_digit(ary[i + 1]) == sum_digit(ary[i])
      print cnt
      print ' '
      puts ary[i]
      cnt += 1
      break if cnt > 10000
    end
  end
}