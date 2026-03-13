\\ ニュートンの恒等式を用いて、異なる k 種類のパーツからなり、
\\ 各パーツの重複数上限が m である分割の母関数（z^k の係数）を計算
get_ek_limit(k, m, n) = {
    if (m <= 0, return(0));
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

fast_A240305(M) = {
    my(total_gf = 1 + O('q^(M+1))); \\ n=0 の 1 を初期値とする
    
    \\ 種類数 k を 2 から回す (k=1 のとき m < 1 は存在しないため)
    for(k = 2, M,
        if(k*(k+1)/2 > M, break);
        
        \\ A240305 の条件: 重複数 m <= k-1
        total_gf += get_ek_limit(k, k-1, M);
    );
    return(total_gf);
}

M=1000;
res = fast_A240305(M);
\\ for(n=0, M, print1(polcoeff(res, n), ", "));
for(n=0, M, write("b240305_fast.txt", n, " ", polcoeff(res, n)));