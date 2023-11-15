a(n) = if(n==0, 1, -3*sum(k=1, n, (-1)^k * binomial(n,k) * a(n-k) ));
for(n=0, 15, print1(a(n),", "))    
