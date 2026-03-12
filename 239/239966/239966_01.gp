default(parisize, 1200000000)

\\ (distinct parts) = (min multiplicity) を計算する関数
part_min_mult(n) = {
    my(q = 'q + O('q^(n+1)));
    my(total_gf = 0);
    
    for(j = 1, n,
        \\ 異なるパーツが j 個で、各重複数が j 以上の最小の n は j^2(j+1)/2
        if(j^2*(j+1)/2 > n, break);
        
        \\ z を変数として、積を計算
        \\ G.f.: Sum_{j>=1} [z^j] ( Product_{k>=1} (1 + z*q^(j*k)/(1-q^k)) - Product_{k>=1} (1 + z*q^((j+1)*k)/(1-q^k)) ).
        my(P = prod(k = 1, n, 1 + 'z * q^(j*k) / (1 - q^k)) - prod(k = 1, n, 1 + 'z * q^((j+1)*k) / (1 - q^k)));
        
        \\ z^j の係数（q のべき級数）を足し合わせる
        total_gf += polcoeff(P, j, 'z);
    );
    
    return(total_gf);
}

M=1000;
res = part_min_mult(M); 
for(n=0, M, write("b239966.txt", n, " ", polcoef(res, n)));