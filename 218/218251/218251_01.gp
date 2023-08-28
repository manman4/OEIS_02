a(n) = sum(k=0, n\3, binomial(n-2*k,k) * binomial(2*n-4*k+3,n-3*k+1)/(2*n-4*k+3) );
for(n=0, 26, print1(a(n), ", "))


