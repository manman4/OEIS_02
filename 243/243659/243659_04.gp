a(n) = (-1)^n*sum(k=0, n, (-2)^(n-k)*binomial(3*n+1, k)*binomial(4*n-k, n-k))/(3*n+1);
for(n=0, 20, print1(a(n),", "))    