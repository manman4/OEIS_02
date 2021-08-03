def A(n)
  b_ary = [[[0, 0], [1, 0], [1, 1], [1, 2]]]
  s = 4
  (3..n).each{|i|
    s += i
    t = 0
    f_ary, b_ary = b_ary, []
    if i % 2 == 1
      f_ary.each{|a|
        b = a.clone
        x, y = *b[-1]
        b += (1..i).map{|j| [x + j, y]}
        b_ary << b if b.uniq.size == s
        t += 1 if b[-1] == [0, 0] && b.uniq.size == s - 1
        c = a.clone
        x, y = *c[-1]
        c += (1..i).map{|j| [x - j, y]}
        b_ary << c if c.uniq.size == s
        t += 1 if c[-1] == [0, 0] && c.uniq.size == s - 1
      }
    else
      f_ary.each{|a|
        b = a.clone
        x, y = *b[-1]
        b += (1..i).map{|j| [x, y + j]}
        b_ary << b if b.uniq.size == s
        t += 1 if b[-1] == [0, 0] && b.uniq.size == s - 1
        c = a.clone
        x, y = *c[-1]
        c += (1..i).map{|j| [x, y - j]}
        b_ary << c if c.uniq.size == s
        t += 1 if c[-1] == [0, 0] && c.uniq.size == s - 1
      }
    end
    p [i, b_ary.size, t]
  }
end

A(10)
