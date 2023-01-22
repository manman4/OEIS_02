M=38;

a(n) = sum(k=0, n\3, binomial(n-2*k, k)*binomial(2*k, k)/(k+1));
for(n=0, M, print1(a(n), ", "));