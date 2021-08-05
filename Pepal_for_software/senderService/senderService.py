from flask import Flask, request
import socket
import requests
import json
import random
import string
from sofTran import *
import argparse
import sys
import subprocess

HYPERLEDGER_ADDRESS = "127.0.0.1"
app = Flask(__name__)

@app.route("/download", methods=['POST'])
def download_request():
    print("post call received")
    req_data = request.get_json()
    if 'txid' not in req_data:
        return 'no txid in POST request'
    txid = req_data['txid']  
    valid_tx, user_ip, user_port = get_status()
    if  valid_tx:
        beginOT(user_ip, user_port)
        return '<h1>OT completed</h1>'
    else:
        print("Abort!")
    return '<h1>txid invalid</h1>'

# gets the token needed to access the blockchain
def send_register_request():
    #randomUName = ''.join([random.choice(string.ascii_letters + string.digits) for n in xrange(5)])
    randomUName = ''.join([random.choice(string.ascii_letters + string.digits) for n in range(5)])
    url = "http://"+ HYPERLEDGER_ADDRESS + ":4000/users"
    headers = {'content-type' : 'application/x-www-form-urlencoded'}
    data = {
        "username": randomUName,
        "orgName": "org1"
    }
    result = requests.post(url, data=data, headers=headers)
    # print result.json()
    if 'token' not in result.json():
        print ('[-] Unable to register/enroll with blockchain. Unable to access hyperledger network')
        return ''
    return result.json()['token']

def convert_to_string(array):
	output = ""
	#for index in xrange(len(array)):
	for index in range(len(array)):
		output += chr(array[index])
	return output

# returns the ip address and port number after verifying the download request transaction
def get_status():    
    accessToken = send_register_request()
    url = "http://"+ HYPERLEDGER_ADDRESS + ':4000/channels/mychannel/chaincodes/mycc'
    headers = {
        'authorization': 'Bearer ' + accessToken, 
        'content-type' : 'application/x-www-form-urlencoded'
    }
    data = {
        "fcn": "get_status",
        "args": ['get_status']
    }
    result = requests.post(url, data=data, headers=headers)
    obj = result.json()
    if "payload" not in obj:
        return False, "0", 0
    data = obj['payload']['data']
    print (accessToken)
    decoded_string = convert_to_string(data)
    print (decoded_string)
    contract = json.loads(decoded_string)
    print (contract)
    print( contract["SoftwareUser"]["IPAddress"] )
    if "SoftwareUser" not in contract:
        return False, "0", 0
    print ('Got User IP address and PORT from transaction')
    return True, contract["SoftwareUser"]["IPAddress"], int(contract["SoftwareUser"]["Port"])


def penalize(hubSig, userSig, message):  
    print ('Invoking penalty with: ')
    print ('Hub signature: ' + hubSig)
    print ('User signature: ' + userSig)
    print ('Message: ' + message)
    accessToken = send_register_request()
    url = "http://"+ HYPERLEDGER_ADDRESS + ':4000/channels/mychannel/chaincodes/mycc'
    headers = {
        'authorization': 'Bearer ' + accessToken, 
        'content-type' : 'application/x-www-form-urlencoded'
    }
    data = {
        "fcn": "penalty",
        "args": ['penalty', message, hubSig, userSig ]
    }
    result = requests.post(url, data=data, headers=headers)
    res = result.json()
    print (res)
    if 'payload' in res:
        print ("[+] Penalize successful. Message: ")
        if 'data' in res['payload']:
            print(convert_to_string(res['payload']['data']) )
    elif 'status' in res:
        if res['status'] != 200:
            print("Penalize did not work")
    else:
        print("[-] Invlaid response from hayperledger")
    
    print ('[+] Penalty call terminated')

# .\watermarkedJARs\HelloWorld0.jar
# .\\test_wm.jar
def watermark(rawFile, outFile, wmBit):
    if wmBit == 0:
        wm = 6666666
    else:
        wm = 8888888
    print( '[+] Watermark called')
    sandmarkPath = '../util/SandMark/sandmark.jar'
    command = "java -cp %s sandmark.smash.SandmarkCLI " % sandmarkPath
    params = " -E -A Monden -i %s -o %s -k %d -w %d" % rawFile, outFile, 4198, wm
    try:
        output = subprocess.run(command+params, shell=True, check=True)
        print('[+] Watermark Successful with key 4198')
        print(output)
    except subprocess.CalledProcessError:
        print("[-] Unable to watermark file: " + rawFile)    
    return

def extractKey(wmFile, exKey):
    print( '[+] Extraction called')
    print (wmFile)
    print (exKey) 
    sandmarkPath = '../util/SandMark/sandmark.jar'
    command = "java -cp %s sandmark.smash.SandmarkCLI" % sandmarkPath
    params = " -R -A Monden -i %s -k 4198" % wmFile
    try:
        output = subprocess.run(command+params, shell=True, check=True)
        print(output)
    except:
        print("[-] Unable to extract watermark")          
    return


def beginOT(user_ip, user_port):
    print("Operating as sender...")
    HOST = "127.0.0.1"
    PORT = 6000
    clt = socket(AF_INET, SOCK_STREAM)
    clt.connect((HOST, PORT))
    clt.settimeout(15)
    msgs = ['one', 'two']
    _name, _type, _sock = "sender", SENDER, clt

    sp = SoftwareTransfer(msgs)
    sp.setup( {'name':_name, 'type':_type, 'socket':_sock} )
    # run as a thread...
    sp.execute(_type)


    # Create a TCP/IP socket
    '''sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Connect the socket to the port where the server is listening
    user_addr = (user_ip, user_port)
    print ('connecting to %s port %s' % user_addr)
    sock.connect(user_addr)
    try:    
        # Send data
        message = 'Some byte[] of a jar file'
        print ('sending "%s"' % message)
        sock.sendall(message)
        data = sock.recv(1024)
        print ('received "%s"' % data)
        print ('OT complete')
    finally:
        print ('closing socket')
        sock.close()
    return'''
def main():
    parser = argparse.ArgumentParser(prog='Sender Service(HUB)')   
    opts = ['preOT', 'penalize', 'status', 'extract', 'watermark'] 
    parser.add_argument('mode', help='before or after OT has been completed', choices=opts)
    parser.add_argument('hubSig', help='signature generated by ECDSASig. format: r,s', nargs='?')
    parser.add_argument('userSig', help='signature generated by ECDSASig. format: r,s', nargs='?')
    parser.add_argument('message', help='message for with the signature was created', nargs='?') 
    args = parser.parse_args()
    if args.mode == opts[1]: # penalty phase
        if not args.hubSig or not args.userSig or not args.message :
            print ('[-] Invalid params for penalize call. Supply signatures and message')
        else:
            penalize(args.hubSig, args.userSig, args.message)
        return
    elif args.mode == opts[2]: # contract status
        get_status()
    elif args.mode == opts[3]: # extract
        if len(sys.argv) < 4:
            print ('[-] Invalid params for extraction call. pass watermarked file name and extraction key')
        else:
            extractKey(sys.argv[2],  sys.argv[3])   
    elif args.mode == opts[4]: # watermark (rawFile, outFile, wmBit)
        if len(sys.argv) < 4:
            print ('[-] Invalid params for watermark call. pass (rawFile, outFile, wmBit)')
        else:
            watermark(sys.argv[2],  sys.argv[3], sys.argv[4])        
    else:
        send_register_request()
        app.run()
    print( '[+] Sender/Hub service terminated')

   

if __name__ == '__main__':
    main()

