part_max_gt_k(n) = {
    my(total_gf = 1);
    my(q = 'q + O('q^(n+1)));
    for(k = 2, n,
        \\ 種類数 k を構成するための最小の n = k(k+1)/2
        if(k*(k+1)/2 > n, break);
        
        \\ 各パーツの重複数 <= k-1
        my(A = polcoeff(prod(j=1, n, 1 + 'z * q^j * (1-q^((k-1)*j))/(1-q^j)), k, 'z));
        total_gf += A;
    );
    return(total_gf);
}

{
    my(res = part_max_gt_k(50));
    for(i=0, 50, print1(polcoeff(res, i), ", "));
}

M=100;
res = part_max_gt_k(M);
\\ for(n=0, M, print1(polcoef(res, n), ", "));
for(n=0, M, write("b240305_slow.txt", n, " ", polcoef(res, n)));