\\ a(0) = 1, a(1) = 0; a(n) = (n-1)*a(n-2) + Sum_{k=1..n-1} a(k) * a(n-1-k).
a(n) = my(v=[1, 0]); if(n<2, v[n+1], (n-1)*a(n-2) + sum(k=1, n-1, a(k)*a(n-1-k)));
for(n=0, 14, print1(a(n), ", "));
