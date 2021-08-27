require 'prime'
require 'OpenSSL'

def A288641(n)
  Prime.each(10 ** 5){|mod|
    x = 2
    (2..mod - 1).each{|i|
      x = OpenSSL::BN.new(i.to_s).mod_inverse(mod).to_i * x * (x ** (n - 1) + i - 1) % mod
    }
    return mod if x * (x ** (n - 1) + mod - 1) % mod > 0
  }
end

n = 1000
(2..n).each{|i| 
  print i
  print ' '
  puts A288641(i)
}
