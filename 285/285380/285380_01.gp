a(n) = my(A=1+O(x)); for(i=1, n, A=1-(n-i+1)!*x/A); polcoef(1/A, n);
M=50;
for(n=0, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b285380_1.txt", n, " ", i))