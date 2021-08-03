def A292793(n)
  ary = [1]
  b_ary = [[[0, 0], [1, 0], [1, 1], [1, 2]]]
  s = 4
  (3..n).each{|i|
    s += i
    f_ary, b_ary = b_ary, []
    if i % 2 == 1
      f_ary.each{|a|
        b = a.clone
        x, y = *b[-1]
        b += (1..i).map{|j| [x + j, y]}
        b_ary << b if b.uniq.size == s
        c = a.clone
        x, y = *c[-1]
        c += (1..i).map{|j| [x - j, y]}
        b_ary << c if c.uniq.size == s
      }
    else
      f_ary.each{|a|
        b = a.clone
        x, y = *b[-1]
        b += (1..i).map{|j| [x, y + j]}
        b_ary << b if b.uniq.size == s
        c = a.clone
        x, y = *c[-1]
        c += (1..i).map{|j| [x, y - j]}
        b_ary << c if c.uniq.size == s
      }
    end
    ary << b_ary.size
  }
  ary
end
p A292793(16)
