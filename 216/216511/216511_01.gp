M=10000;

a(n) = if(n==0, 0, sum(k=1, sqrtint((n-1)\7), issquare(n-7*k^2)));
for(n=0, M, write("b216511_1.txt", n, " ", a(n)));