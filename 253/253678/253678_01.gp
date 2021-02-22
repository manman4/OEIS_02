default(realprecision, 550);
a341533(n, k) = round(sqrt(prod(a=1, n,   prod(b=1, k,   4*sin((2*a-1)*Pi/(2*n))^2+4*sin((2*b-1)*Pi/k)^2))));
a341738(n, k) = round(sqrt(prod(a=1, n,   prod(b=1, k-1, 4*sin((2*a-1)*Pi/(2*n))^2+4*sin(2*b*Pi/k)^2))));
a341739(n, k) = round(     prod(a=1, n-1, prod(b=1, k,   4*sin(a*Pi/n)^2          +4*sin((2*b-1)*Pi/(2*k))^2)));
T(n, k) = a341533(n, k)/2 + a341738(n, k) + 2 * ((k+1)%2) * a341739(n, ceil(k/2));

M=500;
for(n=3, M, i=T(4, n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b253678_1.txt", n, " ", i))