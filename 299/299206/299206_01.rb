def A(n)
  t = [1, -24]
  (2..n).each{|i|
    t[i] = -24 * t[-1] - 2 ** 11 * t[-2]
  }
 p t
end

def B(n)
  t = [1, 4830]
  (2..n).each{|i|
    t[i] = 4830 * t[-1] - 5 ** 11 * t[-2]
  }
 p t
end

n = 100
# A(n)
B(n)
