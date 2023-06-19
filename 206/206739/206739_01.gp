a(n) = {
    my(CF=1+x*O(x^n), M=round((n+1)^(1.0/2))); 
    for(k=0, M, 
        CF=1/(1-x^((M-k+1)^2)*CF)
    ); 
    polcoeff(CF, n, x)
};
for(n=0, 46, print1(a(n),", "))  