a(n) = sum(k=0, n\5, binomial(n-4*k,k) * binomial(3*n-2*k+1,n-4*k)/(3*n-2*k+1) );
for(n=0, 26, print1(a(n), ", "))

