a(n) = (-1)^n*sum(k=0, n, (-2)^(n-k)*binomial(4*n+1, k)*binomial(5*n-k, n-k))/(4*n+1); 
for(n=0, 20, print1(a(n),", "))    