from __future__ import print_function
from __future__ import division
import functools
import operator as op
def ncr(n, r):
    r = min(r, n-r)
    if r == 0: return 1
    numer = functools.reduce(op.mul, range(n, n-r, -1))
    denom = functools.reduce(op.mul, range(1, r+1))
    return numer//denom


def genList(l):
    output = []
    mval = [m for m in range(50,256*l,50)]
    for m in mval:
        ratio = 1- (ncr((k-1)*l,m)/ncr(k*l,m))
        val = k*ratio
        output.append(val)
 #       print("m:%d ratio:%0.2f val:%0.2f " % (m, ratio, val))
    output = [round(elem, 4) for elem in output]
    mval2 = [(m*100)/(256*l) for m in mval]
    mval2 = [round(elem, 4) for elem in mval2]
    return mval2, output

def ot_cord(l):
    output = []
    mval = [m for m in range(50,256*l,50)]
    for m in mval:
        point = m/l
        output.append(point)
    output = [round(elem, 4) for elem in output]
    return output


if __name__=="__main__":
    k = 256
    lList = [2, 3, 4, 8, 16]
    cordList = []
    mvalList, outList = [], []
    for l in lList:
        mval, output = genList(l)
        outList.append(output)
        mvalList.append(mval)
        cord = ot_cord(l)
        cordList.append(cord)





    for i in range(len(mvalList)):
        print("For l val: %d"%(lList[i]))
        a = list(zip(mvalList[i], outList[i]))
        print(*a,  sep=' ')
        print("\n")
        b = list(zip(mvalList[i], cordList[i]))
        print(*b,  sep=' ')
        #print(list(zip(mvalList[i], cordList[i])))
        print("\n\n\n")

 #   print(mvalList)



