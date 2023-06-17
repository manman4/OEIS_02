# x^3 + y^4 = n 
def A(n)
  s = 0
  (1..n).each{|i|
    break if i * i * i > n
    (1..n).each{|j|
      k = i * i * i + j * j * j * j
      break if k > n
      s += 1 if k == n
    }
  }
  s
end

n = 200000
b=[]
(1..n).each{|i| 
 if A(i) > 1
  puts i
  b<<i
 end
}
p b

