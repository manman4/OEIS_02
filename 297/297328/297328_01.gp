\\ A(n,k) = (k/n) * Sum_{j=1..n} A078308(j) * A(n-j,k).

A(n, k) = if(n==0, 1, (k/n) * sum(j=1, n, sumdiv(j, d, d^(j/d+1)) * A(n-j, k)) );
for(n=0, 11, for(k=0, n, print1(A(k, n-k), ", "))); 