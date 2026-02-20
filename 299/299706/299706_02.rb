def f_iter(a, b, c, n)
  stack = [[a, b, c]]
  sum = 0

  until stack.empty?
    a, b, c = stack.pop
    next if a + b + c > n

    sum += n / (a + b + c)

    stack << [ a - 2*b + 2*c,  2*a - b + 2*c,  2*a - 2*b + 3*c ]
    stack << [ a + 2*b + 2*c,  2*a + b + 2*c,  2*a + 2*b + 3*c ]
    stack << [-a + 2*b + 2*c, -2*a + b + 2*c, -2*a + 2*b + 3*c ]
  end

  sum
end

p (1..9).map{|i| f_iter(3, 4, 5, 10 ** i)}

