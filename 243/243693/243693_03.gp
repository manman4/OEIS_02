a(n) = if(n==0, 1, sum(k=0, n-1, 2^k * binomial(n,k) * binomial(3*n-k,n-1-k) )/n); 
for(n=0, 21, print1(a(n),", "))      

