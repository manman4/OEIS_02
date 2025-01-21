\\ a(n) = -Sum_{d|n} (-n/d)^d.
a076717(n) = -sumdiv(n, d, (-n/d)^d);
for(n=1, 20, print1(a076717(n), ", "))

\\ a(0) = 1 and a(n) = (n-1)! * Sum_{k=1..n} A076717(k)*a(n-k)/(n-k)! for n > 0.
a(n) = if(n==0, 1, (n-1)! * sum(k=1, n, a076717(k) * a(n-k) / (n-k)!));
for(n=0, 20, print1(a(n), ", "))
