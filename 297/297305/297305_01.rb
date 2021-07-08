def A(n)
  if n % 5 == 0 || n % 31 == 0 || n % 89 == 0
p n
    return n
  end
  flag = false
  (1..n).each{|a|
    (a..n).each{|b|
      if a ** 4 + b ** 4 < n ** 4
        (b..n).each{|c|
          if a ** 4 + b ** 4 + c ** 4 < n ** 4
            (c..n).each{|d|
              if a ** 4 + b ** 4 + c ** 4 + d ** 4 < n ** 4
                (d..n).each{|e|
                  if a ** 4 + b ** 4 + c ** 4 + d ** 4 + e ** 4 == n ** 4
                    p [n, [a, b, c, d, e]]
                    return true
                  end
                }
              end
            }
          end
        }
      end
    }
  }
  flag
end

n = 57
p (1..n).select{|i| A(i)}
