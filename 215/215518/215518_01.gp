M=30;

a354520(n) = sum(k=1, n\2, (16^k-4^k)*bernfrac(2*k)/(2*k)*binomial(n, 2*k));
a_vector(n) = my(v=vector(n+1)); v[1]=1; for(i=1, n, v[i+1]=sum(j=1, i, a354520(2*j)*binomial(2*i-1, 2*j-1)*v[i-j+1])); v; 
a_vector(M)