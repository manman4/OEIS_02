M=26;

a(n) = if(n==0, 1, -sum(k=0, n-2, binomial(n-1, k)*a(k))); 
for(n=0, M, print1(a(n), ", "));