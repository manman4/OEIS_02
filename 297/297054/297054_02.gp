a010815(n) = if( issquare( 24*n + 1, &n), kronecker(12, n));
\\ a(n) = Sum_{k=0..floor(n/2)} A010815(n-2*k). 
a(n) = sum(k=0, n\2, a010815(n-2*k));
for(n=0, 87, print1(a(n), ", "));