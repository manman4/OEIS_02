\\ For k >= 0, e.g.f. A(x) satisfies A'(x) = exp( A(x)*A'(x)^k ), with A(0)=0.
\\ Let R(x) = Series_Reversion( (1-(1-k*(k+1)*x) * exp(-(k+1)*x))/(k+1)^2 ).
\\ A(x) = R(x) * exp(-k*R(x)).
my(k=0, N=20, x='x+O('x^N), R=serreverse((1-(1-k*(k+1)*x)*exp(-(k+1)*x))/(k+1)^2)); Vec(serlaplace(R*exp(-k*R)))
my(k=1, N=20, x='x+O('x^N), R=serreverse((1-(1-k*(k+1)*x)*exp(-(k+1)*x))/(k+1)^2)); Vec(serlaplace(R*exp(-k*R)))
my(k=2, N=20, x='x+O('x^N), R=serreverse((1-(1-k*(k+1)*x)*exp(-(k+1)*x))/(k+1)^2)); Vec(serlaplace(R*exp(-k*R)))
my(k=3, N=20, x='x+O('x^N), R=serreverse((1-(1-k*(k+1)*x)*exp(-(k+1)*x))/(k+1)^2)); Vec(serlaplace(R*exp(-k*R)))
my(k=4, N=20, x='x+O('x^N), R=serreverse((1-(1-k*(k+1)*x)*exp(-(k+1)*x))/(k+1)^2)); Vec(serlaplace(R*exp(-k*R)))

