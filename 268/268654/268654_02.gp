\\ A(n,0) = 1; A(n,k) = Sum_{x_1,x_2, ..., x_{n+1} >= 0 and x_1 + x_2 + ... + x_{n+1} = k-1} multinomial(k-1; x_1, x_2, ..., x_{n+1}) * Product_{i=1..n} (k - Sum_{j=1..i-1} x_j)^(x_i).
\\ ヘルパー関数：再帰的にループを回す（グローバルな関数として定義）
A_rec(n, k, K, idx, rem, current_sum, current_prod, current_denom) = {
  \\ x_n まで決定したら、最後の x_{n+1} は自動的に残量すべてになる
  if(idx == n + 1,
    my(x_last = rem);
    my(final_denom = current_denom * x_last!);
    my(coeff = K! / final_denom);
    return(coeff * current_prod);
  );
  
  my(total = 0);
  \\ x_idx のループ
  for(x = 0, rem,
    my(next_sum = current_sum + x);
    my(next_prod = current_prod * (k - current_sum)^x);
    my(next_denom = current_denom * x!);
    
    total += A_rec(n, k, K, idx + 1, rem - x, next_sum, next_prod, next_denom);
  );
  
  return(total);
};

\\ メメイン関数
A(n, k) = {
  if(k == 0 || n == 0, return(1));
  return(A_rec(n, k, k - 1, 1, k - 1, 0, 1, 1));
};
for(k=0, 15, print1(A(4, k), ", "));

