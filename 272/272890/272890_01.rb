def f(n)
  ary = []
  (2..n).each{|i|
    if i % 2 == 0
      ary << i if n % i == i / 2
    else
      ary << i if (n % i == (i - 1) / 2) || (n % i == (i + 1) / 2)
    end
  }
  ary
end
def g(ary)
  ary.inject(0){|s, i| s + 1r / i}
end
p (3..10 ** 4).select{|i| (i * g(f(i))).denominator == 1}
