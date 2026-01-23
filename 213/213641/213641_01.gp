\\ a(n) = (1/(n+1)) * Sum_{k=0..floor(n/2)} (2*n-2*k)! * |Stirling1(n-k,n-2*k)|/(n-k)!.
a(n) = sum(k=0, n\2, (2*n-2*k)! * abs(stirling(n-k,n-2*k,1)) / (n-k)! )/(n+1);
for(n=0, 18, print1(a(n),", "))  