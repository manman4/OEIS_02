N = 34
for n in range(N + 1):
    sum = 0
    i = k = 1
    while i<=n:
        product = 1
        for x in range(k):
            product *= i
            i += 1
            if i>n: break
        sum += product
        k += 1
    print(str(n)+' '+str(sum))
