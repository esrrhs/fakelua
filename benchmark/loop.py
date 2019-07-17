
def loop(n):
    count = 0

    for i in xrange(2, n):
        count = count + 1
    return count


N = 10 * 10000000

print(loop(N))
