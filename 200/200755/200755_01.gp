a(n) = sum(k=0, n\2, (-1)^k * binomial(3*n-4*k,k) * binomial(3*n-5*k,n-2*k) / (2*n-3*k+1) ); 
for(n=0, 20, print1(a(n),", ")) 