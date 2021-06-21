def A(n)
  cnt = 0
  (1..n).each{|i|
    (i + 1..n).each{|j|
      if i.gcd(j) == 1
        k = n - i - j
        if k > j
          if j.gcd(k) == 1
            if  i.gcd(k) == 1
            #  p [n, [i, j, k]]
              cnt += 1
            end
          end
        end
      end
    }
  }
  cnt
end

n = 1000
(6..n).each{|i|
  j = A(i)
  break if j.to_s.size > 1000
  print i
  print ' '
  puts j
}
