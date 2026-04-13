# T(1,0) = 1;
# T(n,k) = 2*(k+1)*(2*k+1) * T(n-1,k+1) + 4*((4*k + 1)*n + 2*k^2 - 3*k - 1) * T(n-1,k) + 2*(4*n + 2*k - 3)*(2*n + k - 3) * T(n-1,k-1);
# T(n,k) = 0 if k < 0 or k >= n.

def compute_all_d_values(limit)
  # d[j] は現在の m における d_{m, j}
  # 初期条件 d_{1, 0} = 1
  a = [1]
  ary = [[1]]
  
  (2..limit).each{|n|
    # n ステップ目の全係数 d_{n, k} を表示
    # puts "--- n = #{n} ---"
    a.each_with_index do |val, k|
      # puts "d(#{n}, #{k}) = #{val}"
      # print("#{val}, ")
    end
    # puts "a(#{n}) = #{d[0]}" # これが求める一般項
    # puts ""

    next_a = Array.new(n, 0)
    (0..n-1).each{|k|
      # 1. α係数: d_{n, k+1} からの寄与
      alpha_term = (k + 1 < a.size) ? 2 * (k + 1) * (2 * k + 1) * a[k + 1] : 0
      
      # 2. β係数: d_{n, k} からの寄与
      beta_term = (k < a.size) ? 4 * ((4 * k + 1) * n + 2 * k**2 - 3 * k - 1) * a[k] : 0
      
      # 3. γ係数: d_{n, k-1} からの寄与
      gamma_term = (k > 0) ? 2 * (4 * n + 2 * k - 3) * (2 * n + k - 3) * a[k - 1] : 0
      
      next_a[k] = alpha_term + beta_term + gamma_term
    }
    a = next_a
    ary << a
  }
  ary
end

n = 300
ary = compute_all_d_values(n)
# row[1] のとりだし。ただしnil であれば 0 とする。a(1) = 0, ,,,
(1..n).each{|n|
  a_n = ary[n - 1][0] || 0
  break if a_n.to_s.size > 1000
  print n
  print " "
  puts a_n
}