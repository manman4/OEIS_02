\\ a(n) = (1/((n+1) * 4^(n+1))) * Sum_{k>=0} (3/4)^k * binomial(n+k,k) * binomial(2*(n+k+1),n).
a(n) = sum(k=0, 2000, (3./4)^k * binomial(n+k,k) * binomial(2*(n+k+1), n)) / ((n+1) * 4^(n+1));
for(n=0, 18, print1(a(n), ", "));
for(n=0, 18, print1(round(a(n)), ", "));