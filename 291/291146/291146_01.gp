a(n) = {
    my(CF=1+x*O(x^n), M=round((n+1)^(1.0/3))); 
    for(k=0, M, 
        CF=1/(1-x^((M-k+1)^3)*CF)
    ); 
    polcoeff(CF, n, x)
};
for(n=0, 56, print1(a(n),", "))  