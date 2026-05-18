b(n) = polchebyshev(n-1, 2, 2);
for(n=0, 49, print1(b(n), ", "));

\\ Let b(n) = A001353(n). Then a(n) = 1 + sqrt(1 + b(n-1)*b(n)*b(n+1)*b(n+2)/4) for n > 0.
a(n) = 1 + sqrt(1 + b(n-1)*b(n)*b(n+1)*b(n+2)/4);
for(n=0, 29, print1(a(n), ", "));
for(n=0, 29, print1(round(a(n)), ", "));
