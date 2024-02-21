\\ a(n) = Sum_{k=0..floor(n/3)} (-1)^k * (n-2*k)! * binomial(n-2*k,k). 
a(n) = sum(k=0, n\3, (-1)^k * (n-2*k)! * binomial(n-2*k,k));
for(n=0, 30, print1(a(n), ", "))
