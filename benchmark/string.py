
def loop(n):
    ret = ""
    last = ""

    for i in xrange(2, n):
        ret = last + "ret" + str(i)
        last = "last" + str(i)
    return ret


N = 1000000

print(loop(N))
