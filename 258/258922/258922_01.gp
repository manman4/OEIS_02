\\ a(n) = (1/(n+1)!) * Sum_{k=0..n} 2^k * (n+k)! * Stirling2(n,k). 
a(n) = (1/(n+1)!) * sum(k=0, n, 2^k * (n+k)! * stirling(n, k, 2));
for(n=0, 20, print1(a(n),", "))   
