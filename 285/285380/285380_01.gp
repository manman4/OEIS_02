a(n) = local(A=1+x*O(x^n)); for(i=1, n, A=1-(n-i+1)!*x/A); polcoeff(1/A, n); 
M=50;
for(n=0, M, i=a(n); if((i<0)+#digits(i)>1000, break); write("/Users/xxx/Desktop/b285380_1.txt", n, " ", i))