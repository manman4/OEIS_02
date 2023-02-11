M=20;

a(n) = sum(k=0, n, k^(n-k)*(k+2)^(k-1)*binomial(n, k))/2^(n-1); 
for(n=0, M, print1(a(n), ", ")) 
