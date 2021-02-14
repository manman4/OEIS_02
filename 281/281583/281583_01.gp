default(realprecision, 120);

a(n) = round(sqrt(prod(j=1, n, prod(k=1, 9, 4*sin((2*j-1)*Pi/(2*n))^2+4*sin((2*k-1)*Pi/9)^2))));
for(n=2, 20, print1(a(n), ", "))