\\ ニュートンの恒等式を用いて、異なる k 種類のパーツからなり、
\\ 各パーツの重複数上限が m である分割の母関数（z^k の係数）を直接計算する
get_ek_limit(k, m, n) = {
    my(S = vector(k));
    my(q = 'q + O('q^(n+1)));
    
    \\ べき乗和 S_i = sum_{j=1}^{n} (q^j + q^{2j} + ... + q^{mj})^i
    for(i = 1, k,
        S[i] = sum(j = 1, n\i, (q^j * (1 - q^(m*j)) / (1 - q^j))^i);
    );
    
    \\ 基本対称式 e_k を計算
    my(e = vector(k+1));
    e[1] = 1;
    for(i = 1, k,
        e[i+1] = sum(j = 1, i, (-1)^(j-1) * e[i-j+1] * S[j]) / i;
    );
    return(e[k+1]);
}

fast_A240306(M) = {
    my(total_gf = 1);
    my(q = 'q + O('q^(M+1)));
    
    for(k = 1, M,
        if(k*(k+1)/2 > M, break);
        
        \\ A240306 の条件: 種類数 k に対して 最大重複数 <= k
        total_gf += get_ek_limit(k, k, M);
    );
    return(total_gf);
}

M=1000;
res = fast_A240306(M);
\\ for(n=0, M, print1(polcoef(res, n), ", "));
for(n=0, M, write("b240306_fast.txt", n, " ", polcoef(res, n)));