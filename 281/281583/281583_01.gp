\\ \r
\\ Nは使わない 

default(realprecision, 1500);

a(n) = round(sqrt(prod(j=1, n, prod(k=1, 9, 4*sin((2*j-1)*Pi/(2*n))^2+4*sin((2*k-1)*Pi/9)^2))));

M=500;
for(n=2, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b281583_2.txt", n, " ", i))
