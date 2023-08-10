a(n) = if(n==0, 1, sum(k=1, n, 3^k * 2^(n-k) * binomial(n,k) * binomial(2*n,k-1) )/n); 
for(n=0, 19, print1(a(n),", "))          
