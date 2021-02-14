def B(p)
  a, b = 50, 224
  cnt = 2
  ary = [50]
  while cnt < p
    a, b = b, 5*b - a -12
    ary << a
    cnt += 1
  end
  ary
end


n = 20
p B(n)


