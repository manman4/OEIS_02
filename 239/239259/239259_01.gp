a000041(n) = numbpart(n);
a000009(n) = if(n<0, 0, polcoeff( prod( k=1, n, 1 + x^k, 1 + x * O(x^n)), n));

\\ a(2*n+m) = Sum_{k=0..floor(n-1)/2} A000009(2*k+m) * A000041(n-k) for m=0, 1.
a(n) = my(n1=n\2); if(n%2==0, sum(k=0, (n1-1)\2, a000009(2*k) * a000041(n1-k)), sum(k=0, (n1-1)\2, a000009(2*k+1) * a000041(n1-k)));
for(n=0, 49, print1(a(n), ", "));