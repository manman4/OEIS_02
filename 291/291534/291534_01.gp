a(n) = sum(k=0, n-1, (-1)^k*binomial(n, k)*binomial(2*n, n-1-k))/n; 
for(n=1, 20, print1(a(n),", "))     