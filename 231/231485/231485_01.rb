def B(p)
  a, b, c, d, e, f, g, h= 722, 9922, 155682, 2540032, 41934482, 694861522, 11527389122, 191304901282
  cnt = 2
  ary = [722]
  while cnt < p
    a, b, c, d, e, f, g, h = b, c, d, e, f, g, h, 28*h - 233*g + 796*f - 1205*e + 796*d - 233*c + 28*b - a + 200
    ary << a
    cnt += 1
  end
  ary
end


n = 20
p B(n)


