require 'prime'

def d(m, k)
  return m unless @factor[k]
  (0..@factor[k][1]).map{|e| d(m * @factor[k][0] ** e, k + 1)}.flatten
end

(0..17).each{|i|
  n = 4 * 10 ** i + 3
  # ���Ԃ��t�ɂ��������̌��ʂ��Y��ɂȂ�
  @factor = Prime.prime_division(n).reverse
  s = 0
  d(1, 0).each{|j|
    k = j % 4
    if k == 3
      s += j * j
    elsif k == 1
      s -= j * j
    end
  }
  print i
  print ' ' 
  puts s / 8
}
