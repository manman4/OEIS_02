default(parisize, 1200000000)

\\ ニュートンの恒等式を用いて、各パーツが最大 m 回まで重複可能な
\\ 「異なる k 種類のパーツからなる分割」の母関数を計算する
get_ek(k, m, n) = {
    my(S = vector(k));
    my(q = 'q + O('q^(n+1)));
    
    \\ 1. べき乗和 S_i を計算
    \\ S_i = sum_{j=1}^{n} (q^j * (1-q^{mj})/(1-q^j))^i
    for(i=1, k,
        S[i] = sum(j=1, n\i, (q^j * (1-q^(m*j)) / (1-q^j))^i);
    );
    
    \\ 2. 基本対称式 e_k を再帰的に求める (Newton's sums)
    my(e = vector(k+1));
    e[1] = 1; \\ e_0 = 1
    for(i=1, k,
        my(term = sum(j=1, i, (-1)^(j-1) * e[i-j+1] * S[j]));
        e[i+1] = term / i;
    );
    return(e[k+1]);
}

fast_part_max_eq_k(M) = {
    my(total_gf = 0);
    my(q = 'q + O('q^(M+1)));
    
    for(k = 1, M,
        if(k*(k+1)/2 > M, break); \\ 最小の和の下限チェック
        
        my(A = get_ek(k, k, M));
        my(B = 0);
        if(k > 1,
            B = get_ek(k, k-1, M);
        );
        total_gf += (A - B);
    );
    return(total_gf);
}

M=1000;
res = fast_part_max_eq_k(M);
\\ for(n=0, M, print1(polcoeff(res, n), ", "));
for(n=0, M, write("b239964_fast.txt", n, " ", polcoeff(res, n)));