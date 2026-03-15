\\ G.f.: Sum_{j>=1} (q^(j^2) + Sum_{k=1..j-1} q^((j+1)*k)) / Product_{k=1..j-1} (1-q^k).
my(N=50, q='q+O('q^N)); sum(j=1, N, (q^(j^2) + sum(k=1, j-1, q^((j+1)*k)))/prod(k=1, j-1, 1-q^k))
my(N=50, q='q+O('q^N)); concat(0, Vec(sum(j=1, N, (q^(j^2) + sum(k=1, j-1, q^((j+1)*k)))/prod(k=1, j-1, 1-q^k))))