\\ a(n) = Sum_{k divides n} (n/k)^k.
a055225(n) = sumdiv(n, k, (n/k)^k);
for(n=1, 20, print1(a055225(n), ", "))

\\ a(0) = 1 and a(n) = (n-1)! * Sum_{k=1..n} A055225(k)*a(n-k)/(n-k)! for n > 0.
a(n) = if(n==0, 1, (n-1)! * sum(k=1, n, a055225(k) * a(n-k) / (n-k)!));
for(n=0, 20, print1(a(n), ", "))
