a(n) = sum(k=0, n, (-1)^(n-k) * binomial(n+3*k+1, k) * binomial(k, n-k)/(n+3*k+1)); 

for(n=0, 15, print1(a(n),", ")) 