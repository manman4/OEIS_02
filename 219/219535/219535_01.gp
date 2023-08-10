a(n) = sum(k=0, n, 2^(n-k)*binomial(n, k)*binomial(2*n+k+1, n)/(2*n+k+1));
for(n=0, 19, print1(a(n),", "))          
