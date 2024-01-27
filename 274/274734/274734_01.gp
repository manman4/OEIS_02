\\ a(n) = (1/(n+1)) * Sum_{k=0..n} binomial(n+k,k) * binomial(2*n+2*k+2,n-k).
a(n) = (1/(n+1)) * sum(k=0, n, binomial(n+k,k) * binomial(2*n+2*k+2,n-k));
for(n=0, 25, print1(a(n),", "))   
      