my(N=50, x='x+O('x^N)); concat(0, Vec(1/prod(k=1, N, 1-x^k) * sum(k=1, N,  (-1)^(k-1) * x^(k*(3*k+1)/2) / (1-x^k)) ))   
