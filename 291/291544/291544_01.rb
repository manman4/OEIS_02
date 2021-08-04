require 'prime'
def A(n)
  ary = [2]
  sum = 2
  p_ary = [sum]
  s = 1
  while ary.size < n
    if !ary.include?(s)
      if (sum + s).prime?
        ary << s
        sum += s
        p_ary << sum
        s = 0
      end
    end
    s += 1
  end
  p_ary
end

n = 1000
ary = A(n)
(1..n).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}
