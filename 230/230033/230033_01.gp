\\ \r
\\ Nは使わない 

default(realprecision, 1200);

a(n) = round(sqrt(prod(j=1, n, prod(k=1, 7, 4*sin((2*j-1)*Pi/(2*n))^2+4*sin((2*k-1)*Pi/7)^2))));

M=500;
for(n=2, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b230033_1.txt", n, " ", i))