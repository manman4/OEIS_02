\\ a(0) = 1; a(n) = Sum_{j=0..floor((n-1)/2)} a(n-1-2*j) * Sum_{k=0..j} a(k)*a(j-k).
a(n) = if( n==0, 1, sum(j=0, (n-1)\2, a(n-1-2*j) * sum(k=0, j, a(k)*a(j-k))));
for(n=0, 15, print1(a(n), ", "));

