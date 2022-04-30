M=21;

a_vector(n) = my(v=vector(n)); v[1]=1; for(i=2, n, v[i]=(i%2)*(i-1)!+sum(j=1, i\2, (2*j-2)!*binomial(i-1, 2*j-1)*v[i-2*j+1])); v;
a_vector(M)