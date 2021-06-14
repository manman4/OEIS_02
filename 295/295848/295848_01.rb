class Array
  def gcd
    self.inject{|a, b| a.gcd(b)}
  end
end

def f(n)
  s = 0
  0.upto(n){|x|
    0.upto(n){|y|
      0.upto(n){|z|
        if [x, y, z].gcd == 1 && x * x + y * y + z * z == n
          s += 1
        end
      }
    }
  }
  s
end

n = 200
(0..n).each{|i|
  print i
  print ' '
  puts f(i)
}
