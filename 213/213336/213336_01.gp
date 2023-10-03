a(n) = sum(k=0, n, binomial(n+3*k-1,n-k) * binomial(4*k,k)/(3*k+1) );                         
for(n=0, 17, print1(a(n),", "))    

b(n) = sum(k=0, n, a(k));
for(n=0, 17, print1(b(n),", "))