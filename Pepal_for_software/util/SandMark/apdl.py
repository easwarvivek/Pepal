import subprocess
import sys
sandmarkPath = './sandmark.jar'
zeroWm = "6666666"
oneWm = "8888888"

def watermark(rawFile, outFile, wmBit):
    if wmBit == 0:
        wm = zeroWm
    else:
        wm = oneWm
    print( '[+] Watermark called')    
    command = "java -cp %s sandmark.smash.SandmarkCLI " % sandmarkPath
    params = " -E -A Monden -i %s -o %s -k %d -w %s" % (rawFile, outFile, 4198, wm)
    try:
        output = subprocess.run(command+params, shell=True, check=True)
        print('[+] Watermark Successful with key 4198')
        print(output)
    except subprocess.CalledProcessError:
        print("[-] Unable to watermark file: " + rawFile)    
    return

def extractBit(wmFile, exKey):
    command = "java -cp %s sandmark.smash.SandmarkCLI" % sandmarkPath
    #command = "java -cp "+ sandmarkPath +  " sandmark.smash.SandmarkCLI" 
    print (command)
    params = " -R -A Monden -i %s -k 4198" % wmFile
    #params = " -R -A Monden -i " + wmFile +  " -k 4198" 
    print (params)
    print ("command+params", command+params)
    try:
        result = subprocess.run(command+params, shell=True, check=True, stdout=subprocess.PIPE)
        print(result)
        detected = str(result.stdout)
        print(detected)
        if zeroWm in detected:
            wm = "0"
        elif oneWm in  detected:
            wm = "1"
        else:
            wm = detected
        print("[+] Watermark : " + wm)
    except:
        print("[-] Unable to extract watermark")          
    return


def main():
    print( '[+] Verify sandmark.jar is in: ' + sandmarkPath)
    if len(sys.argv) == 2:
        extractBit(sys.argv[1],  "4198") # (wmFile)
    elif len(sys.argv) == 4: 
        watermark(sys.argv[1],  sys.argv[2], int(sys.argv[3]) ) # (rawFile, outFile, wmBit)
    else:
        print ('[-] Invalid params for extraction/wm call.')

main()
