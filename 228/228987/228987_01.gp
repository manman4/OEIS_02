a(n) = sum(k=0, n\3, binomial(n-2*k,k) * binomial(3*n-k+1,n-2*k)/(3*n-k+1) );
for(n=0, 23, print1(a(n), ", "))

