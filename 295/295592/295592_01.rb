require 'prime'

def f(m, n)
  cnt = 0
  i = 2
  while cnt < n
    i += 1
    ary = []
    (1..Math.sqrt(i).to_i).each{|j|
      if i % j == 0
        ary << j
        ary << i / j
      end
    }
    ary.uniq!
    s = 1
    ary.each{|i| s *= i + 1 if (i + 1).prime?}
    if s == m
      cnt += 1
      print cnt
      print ' '
      puts i
    end
  end
end

f(64722, 1000)
