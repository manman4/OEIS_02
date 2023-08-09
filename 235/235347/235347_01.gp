a(n) = (-1)^n * sum(k=0, n, (-3)^k * binomial(n,k) * binomial(2*n+k+1,n) / (2*n+k+1) ); 
for(n=0, 20, print1(a(n),", "))   

