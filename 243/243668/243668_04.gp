a(n) = if(n==0, 1, sum(k=0, n-1, binomial(n,k) * binomial(6*n-k,n-1-k) )/n);            
for(n=0, 20, print1(a(n),", "))    