\\ For k > 0, with V(x) = LambertW(e^(1/k) * (1-(k+1)^2*x)/k), we have R(x) = (1-k*V(x))/(k*(k+1)) and A(x) = (1-k*V(x))/(k*(k+1)) * exp((k*V(x)-1)/(k+1)). 
my(k=1, N=20, x='x+O('x^N), V=lambertw(exp(1/k)*(1-(k+1)^2*x)/k)); apply(round, Vec(serlaplace((1-k*V)/(k*(k+1)) * exp((k*V-1)/(k+1)))))
my(k=2, N=20, x='x+O('x^N), V=lambertw(exp(1/k)*(1-(k+1)^2*x)/k)); apply(round, Vec(serlaplace((1-k*V)/(k*(k+1)) * exp((k*V-1)/(k+1)))))
my(k=3, N=20, x='x+O('x^N), V=lambertw(exp(1/k)*(1-(k+1)^2*x)/k)); apply(round, Vec(serlaplace((1-k*V)/(k*(k+1)) * exp((k*V-1)/(k+1)))))
my(k=4, N=20, x='x+O('x^N), V=lambertw(exp(1/k)*(1-(k+1)^2*x)/k)); apply(round, Vec(serlaplace((1-k*V)/(k*(k+1)) * exp((k*V-1)/(k+1)))))

my(k=1, N=20, x='x+O('x^N), V=lambertw(exp(1/k)*(1-(k+1)^2*x)/k)); serlaplace((1-k*V)/(k*(k+1)) * exp((k*V-1)/(k+1)))