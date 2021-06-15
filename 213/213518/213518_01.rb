def A(n)
  d = 0
  a = (0..9).map{|i| i}
  ary = [4, 5, 6, 7, 8, 9]
  while d < n - 1
    d += 1
    # p [d, a.size]
    b = []
    a.each{|i|
      (0..9).each{|j|
        k = j * 10 ** d + i
        t = k * (k + 1) / 2
        if t >= 10 ** (2 * d - 1)
          l0 = ( t                % (10 ** (d + 1))).to_s.rjust(d + 1, "0").split('').to_a.uniq.size
          l1 = ((t + 5 * 10 ** d) % (10 ** (d + 1))).to_s.rjust(d + 1, "0").split('').to_a.uniq.size
          if l0 <= 2 || l1 <= 2
            b << k
            ary << k if t.to_s.split('').to_a.uniq.size == 2
          end
        end
      }
    }
    a = b.uniq
  end
  ary.uniq.sort
end

ary = A(7)
(1..ary.size).each{|i|
  print i
  print ' '
  puts ary[i - 1]
}


#p (0..10000).map{|i| (i * (i + 1) / 2) % 1000}.uniq
