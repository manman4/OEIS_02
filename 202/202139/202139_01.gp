M=21;

a_vector(n) = my(v=vector(n+1)); v[1]=0; for(i=1, n, v[i+1]=(i%2)*(i-1)!+sum(j=1, i\2, (2*j-2)!*binomial(i-1, 2*j-1)*v[i-2*j+2])); v; 
a_vector(M)