\\ a(n) = (2*n)! * (H(2*n) - H(n)) where H(n) = Sum_{k=1..n} 1/k.
a(n) = (2*n)!*sum(k=1, n, 1/(n+k));
for(n=1, 20, print1(a(n),", "));


