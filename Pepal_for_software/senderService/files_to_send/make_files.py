
def write_files():
    fp0 = open('HelloWorld0_wm0.jar','rb')
    fp1 = open('HelloWorld0_wm1.jar','rb')
    content0 = fp0.read()
    content1 = fp1.read()

    for i in range(256):
        filename0 = "./HelloWorld"+ str(i)+"_wm0.jar"
        filename1 = "./HelloWorld"+ str(i)+"_wm1.jar"
        fh0 = open(filename0, 'wb+')
        fh1 = open(filename1, 'wb+')
        fh0.write(content0)
        fh1.write(content1)
        fh0.close()
        fh1.close()

if __name__ == '__main__':
    write_files()

