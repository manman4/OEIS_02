# def A(n)
#   m = Math.sqrt(n).to_i
#   ary = Array.new(n + 1, 0)
#   (1..m).each{|i|
   
#     sum = i * i
#     ary[sum] += 1
#     i += 1
    
#     sum += i * i
#     while sum <= n
#       j = ary[sum]
#       ary[sum] += 1
#       i += 1
      
#       sum += i * i
#     end
#   }
#   ary
# end

# p A(500)
# p (7..18).inject(0){|s, i| s + i * i}

# def B(n)
#   ary = Array.new(n + 1, 0)
#   (1..n).each{|i|
    
#     sum = i
#     ary[sum] += 1
#     i += 1
    
#     sum += i
#     while sum <= n
#       j = ary[sum]
#       ary[sum] += 1
#       i += 1
     
#       sum += i
#     end
#   }
#   ary
# end

# p B(500)

def C(n)
  m = (n ** (1.0 / 3)).to_i
  ary = Array.new(n + 1, 0)
  (1..m).each{|i|
   
    sum = i * i * i
    ary[sum] += 1
    i += 1
    
    sum += i * i * i
    while sum <= n
      j = ary[sum]
      ary[sum] += 1
      # p sum
      i += 1
      
      sum += i * i * i
    end
  }
  ary
end

p C(500)
