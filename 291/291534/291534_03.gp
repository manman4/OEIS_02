a(n) = sum(k=0, n-1, (-2)^k * binomial(n,k) * binomial(3*n-k,n-1-k) )/n; 
for(n=1, 19, print1(a(n),", "))   
