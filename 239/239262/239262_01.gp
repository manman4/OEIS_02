a000041(n) = numbpart(n);
a000009(n) = if(n<0, 0, polcoeff( prod( k=1, n, 1 + x^k, 1 + x * O(x^n)), n));

\\ a(n) = Sum_{k=0..floor((n-1)/4)} A000009(n-2*k) * A000041(k).
a(n) = sum(k=0, (n-1)\4, a000009(n-2*k) * a000041(k));
for(n=0, 48, print1(a(n), ", "));