N=5000;

for(k=1, N, for(j=1, k, for(i=1, j, if(i*j>k, break); if(i^2+j^2+k^2==3*i*j*k, print1(i, ", ", j, ", ", k, ", ")))));