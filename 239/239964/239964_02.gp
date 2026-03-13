default(parisize, 1200000000)

part_max_eq_k(n) = {
    my(total_gf = 0);
    for(k = 1, n,
        if(k*(k+1)/2 > n && k > 1, break); \\ 種類数 k を作る最小の n は k(k+1)/2
        
        my(A = polcoeff(prod(j=1, n, 1 + z * q^j * (1-q^(k*j))/(1-q^j) + O(q^(n+1))), k, z));
        my(B = 0);
        if(k > 1,
            B = polcoeff(prod(j=1, n, 1 + z * q^j * (1-q^((k-1)*j))/(1-q^j) + O(q^(n+1))), k, z);
        );
        total_gf += (A - B);
    );
    return(total_gf);
}

M=100;
res = part_max_eq_k(M);
\\ for(n=0, M, print1(polcoeff(res, n), ", "));
for(n=0, M, write("b239964.txt", n, " ", polcoeff(res, n)));