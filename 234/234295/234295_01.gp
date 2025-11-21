\\ a(0) = 1; a(n) = a(n-1) + 4 * Sum_{k=1..n-1} binomial(n-1,k) * a(k) * a(n-k).
a(n) = if(n==0, 1, a(n-1) + 4 * sum(k=1, n-1, binomial(n-1,k) * a(k) * a(n-k)));
for(n=0, 14, print1(a(n),", "))  