a(n) = if(n==0, 1, 3*a(n-1) + 2*sum(k=1, n-1, binomial(n-1,k) * a(n-k) ));
for(n=0, 17, print1(a(n),", "))    
