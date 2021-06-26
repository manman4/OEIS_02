require 'prime'

def A(n)
  ary = []
  i = 1
  while ary.size < n
    i_ary = i.prime_division.to_a
    if i_ary.size == 2
      if i_ary[0][1] == 1 && i_ary[1][1] == 1
        j_ary = (i + 1).prime_division.to_a
        if j_ary.size == 2
          if j_ary[0][1] == 1 && j_ary[1][1] == 1
            ary << i
          end
        end
      end
    end
    i += 1
  end
  ary
end

n = 62
ary = A(n)
(1..n).each{|i|
  j = ary[i - 1]
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
