from charm.core.engine.protocol import *
import socket               # Import socket module
import requests
from sofTran import *
HYPERLEDGER_ADDRESS = "127.0.0.1"

# gets the token needed to access the blockchain
# do not use. For testing
def send_register_request():
    randomUName = ''.join([random.choice(string.ascii_letters + string.digits) for n in xrange(5)])
    url = "http://"+ HYPERLEDGER_ADDRESS + ":4000/users"
    headers = {'content-type' : 'application/x-www-form-urlencoded'}
    data = {
        "username": randomUName,
        "orgName": "org1"
    }
    result = requests.post(url, data=data, headers=headers)
    print (result.json())
    return result.json()['token']

# do not use. For testing
def makeDownloadRequest():
    accessToken = send_register_request()
    # REST call to blockchain to see details of the transaction
    url = "http://" + HYPERLEDGER_ADDRESS + ":4000/channels/mychannel/chaincodes/mycc"
    print (accessToken)
    headers = {
        'Content-Type' : 'application/x-www-form-urlencoded',
        'Authorization': 'Bearer ' + accessToken 
    }
    data = {
        "fcn": "download_request",
        "args": ["sender_pk", "recipient_pk", "5000", "05/05/2019", "127.0.0.1", "6000"]
    }
    result = requests.post(url, data=data, headers=headers)
    #print result.text
    print (result.text)

def getDocument():
 #   s = socket.socket()         # Create a socket object
    HOST = 'localhost' # Get local machine name
    PORT = 6000                # Reserve a port for your service.
    print("Operating as receiver...")
    svr = socket(AF_INET, SOCK_STREAM)
    svr.bind((HOST, PORT))
    svr.listen(1)
    svr_sock, addr = svr.accept()
    print("Connected by ", addr)
    msgs = None
    _name, _type, _sock = "receiver", RECEIVER, svr_sock

    sp = SoftwareTransfer(msgs)
    sp.setup( {'name':_name, 'type':_type, 'socket':_sock} )
    # run as a thread...
    sp.execute(_type)

    '''s.bind((host, port))        # Bind to the port

    s.listen(5)                 # Now wait for client connection.
    while True:
        # Wait for a connection
        #print 'waiting for a connection on ' + host + ":" + str(port)
        connection, owner_addr = s.accept()
        try:
            #print 'connection from', owner_addr
            # Receive the data in small chunks and retransmit it
            data = connection.recv(1024)
            #print 'received "%s"' % data
            if data:
                #print 'received something from the hub'
                connection.sendall('Received everything - OT complete')                
        finally:
            connection.close()'''

def main():
    getDocument()

if __name__ == '__main__':
    main()
