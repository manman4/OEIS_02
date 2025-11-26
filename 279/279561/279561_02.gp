\\ a(n) = (1/(2*n+1)) * Sum_{k=0..floor(n/3)} (6*k+1) * binomial(2*n+1,n-3*k). 
a(n) = sum(k=0, n\3, (6*k+1)*binomial(2*n+1, n-3*k)/(2*n+1));
for(n=0, 18, print1(a(n), ", "));