\\ A228873 a(n) = F(n) * F(n+1) * F(n+2) * F(n+3)
a228873(n) = fibonacci(n) * fibonacci(n+1) * fibonacci(n+2) * fibonacci(n+3);
for(n=1, 49, print1(a228873(n), ", "));

\\ a(n) = sqrt(1 + 4*F(n-1)*F(n)*F(n+1)*F(n+2)) = sqrt(1 + 4*A228873(n-1)). 
a(n) = sqrt(1 + 4*a228873(n-1));
for(n=0, 49, print1(a(n), ", "));
for(n=0, 49, print1(round(a(n)), ", "));
