'''
Protocol that implements a Smart NDA 

Author: Easwar Vivek Mangipudi
Date: March 7, 2018
'''

from charm.core.engine.protocol import *
from charm.core.engine.util import *

from charm.toolbox.pairinggroup import PairingGroup,ZR,G1,G2,GT,pair
from charm.toolbox.ecgroup import ECGroup
from charm.core.math.integer import integer
from charm.toolbox.eccurve import prime192v1, prime192v2
from socket import *

from hashlib import sha256
from charm.toolbox.symcrypto import SymmetricCryptoAbstraction
from charm.toolbox.conversion import Conversion
from charm.toolbox.conversion import *
from charm.core.crypto.cryptobase import AES, MODE_CBC

import sys 
import json
import hashlib
import charm.toolbox.symcrypto
import zipfile

#import oblivious_transfer_simple

DEBUG = False
#DEBUG = True

SENDER,RECEIVER = 1,2 
HOST, PORT = "127.0.0.1", 50008

def SHA2(bytes1):
    s1 = hashlib.new('sha256')
    s1.update(bytes1)
    return s1.digest()


def generatePoints(row, col, group):
    groupPoints = [ [] for x in range(row)]
    for i in range(row):
        for j in range(col):
            groupPoints[i].append(group.random(G1))

    return groupPoints
       
def ElGamalEnc(values, pk, group):
    row = len(values)
    col = len(values[0])
    randomVal = [[] for x in range(row)]
    encVal = [[] for x in range(row)]
    for i in range(row):
        for j in range(col):
            randomVal[i].append(  group.random(ZR) )
            second_coord = values[i][j] * (pk  ** randomVal[i][j])
            encVal[i].append(second_coord )
    return randomVal , encVal
    
def ElGamalEnc2(values, pk, group):
    row = len(values)
    col = len(values[0])
    randomVal = []
    encVal = [[] for x in range(row)]
    for i in range(row):
        randomVal.append(  group.random(ZR) )
        for j in range(col):
            second_coord = values[i][j] * (pk  ** randomVal[i])
            encVal[i].append(second_coord )
    return randomVal , encVal

def ReRandElGamal(values, pk, group):
    row = len(values)
    randomVal = []
    encVal = []
    for i in range(row):
            randomVal.append(  group.random(ZR) )
            second_coord = values[i] * (pk  ** randomVal[i])
            encVal.append(second_coord )
    return randomVal , encVal

def show_jar_classes(jar_file):
    """prints out .class files from jar_file"""
    count = 0
    zf = zipfile.ZipFile(jar_file, 'r')
    try:
        lst = zf.infolist()
        for zi in lst:
            fn = zi.filename
            if fn.endswith('.class'):
                count = count + 1
                print(fn)
    finally:
        zf.close()
        return count


class SoftwareTransfer(Protocol):
    def __init__(self, messages=None, groupObj=None, common_input=None):
        Protocol.__init__(self, None)
        receiver_states = { 2:self.receiver_init_transfer2, 4:self.receiver_transfer4, 6:self.recv_ot6, 8:self.final_recv8, 10:self.recv_encryptions10, 12:self.rec_enc_part2_12, 14:self.rec_enc_part3_14 , 16:self.rec_enc_part4_16    }
        sender_states = { 1:self.sender_init_transfer1, 3:self.sender_transfer3, 5:self.init_ot5, 7:self.sender_dec_layer7, 9:self.send_encryptions9, 11:self.send_enc_part2_11, 13:self.send_enc_part3_13, 15:self.send_enc_part4_15 }

        sender_trans   = {1:3, 3:5, 5:7, 7:9, 9:11, 11:13, 13:15}
        receiver_trans = {2:4, 4:6, 6:8, 8:10, 10:12,12:14, 14:16}
        # describe the parties involved and the valid transitions
        Protocol.addPartyType(self, SENDER, sender_states, sender_trans, True)
        Protocol.addPartyType(self, RECEIVER, receiver_states, receiver_trans)
        Protocol.setSerializers(self, self.serialize, self.deserialize)
        # proof parameter generation
        
        if groupObj == None:
            self.group = PairingGroup('SS512')
            #self.group = ECGroup(prime192v1)
        else:
            self.group = groupObj

        if common_input == None: # generate common parameters to P and V
            print("Setting up database")
            db = {}
            self.__gen_setup = True
        else: # can be used as a sub-protocol if common_input is specified by caller
            db = common_input
            self.__gen_setup = False
        #Protocol.setSubclassVars(self, db)
        Protocol.setSubclassVars(self, self.group, db)
        if messages != None:
            self.M, self.sig = [], []
            for i in range(0, len(messages)):
                self.M.append( bytes(messages[i], 'utf8') )
                print("bytes =>", self.M[i],", message =>", messages[i])


    def get_common(self):
        if self.__gen_setup:
            g = self.group.random(G1)
            H = self.group.hash
            Protocol.store(self, ('g', g), ('H', H) )
            return (g,H)
        else: # common parameters generated already
            return Protocol.get(self, ['g', 'H'])


    def sender_init_transfer1(self):
        print("SENDER 1: ")
        (g,H) = self.get_common()
        sender_sk = self.group.random(ZR)
        sender_pk = g ** sender_sk
        S = {'g':g,'sender_pk':sender_pk}
        
        Protocol.store(self, ('sender_pk', sender_pk),('sender_sk',sender_sk),('g',g) )
        Protocol.setState(self, 3)
        print("g =>", g)
        return {'S':S}


    def receiver_init_transfer2(self, input):
        print("RECEIVER 2:")
        S  = Protocol.get(self, ['S'])[0]
        g = S['g']
        sender_pk = S['sender_pk']
        #receiver_sk = self.group.random(ZR)
        
        key_file = open("key.txt",'r')
        receiver_sk = int(key_file.read())
        '''print("type of key", type(key))
        print("Key read from file:", key)
        rpk = g** int(key)
        print("Works, rpk",rpk)'''


        print ("Receiver sk:", receiver_sk)
        receiver_pk = g ** receiver_sk
        common_pk = sender_pk ** receiver_sk
        print("common_pk:", common_pk)
        Protocol.store(self, ('sender_pk', sender_pk),('receiver_sk',receiver_sk),('receiver_pk', receiver_pk),('common_pk', common_pk),('g',g) )
        S2 = {'receiver_pk':receiver_pk, 'common_pk':common_pk}
        Protocol.setState(self, 4)
        return {'S2':S2}

        

    def sender_transfer3(self, input):
        print("SENDER 3: ")
        M = self.M 
        S2  = Protocol.get(self, ['S2'])[0]
        g = Protocol.get(self, ['g'])[0]
        print("g", g)
        common_pk = S2['common_pk']
        #messages = img_watermark()
        row = 256
        col = 2
        groupPoints = generatePoints(row, col, self.group)
        if DEBUG:
            for i in range(3):
                print(groupPoints[i])
        
        hashedPoints = [[] for x in range(row)]
        for i in range(row):
            for j in range(col):
                hashedPoints[i].append(SHA2(objectToBytes(groupPoints[i][j],self.group)))
        print("Size of groupPoints:", len(groupPoints) )
        print("Size of hashedPoints:", len(hashedPoints) )
        randomVals, elgPoints = ElGamalEnc2(groupPoints, common_pk, self.group)
        print("Size of encryptedPoints:", len(elgPoints) )
        #print("encryptedPoints:", encryptedPoints)

        a = []
        A = []
        for i in range(row):
            a.append(  self.group.random(ZR) )
            A.append( g ** a[i] )
        S3 = {'elgPoints':elgPoints, 'A':A} 
        Protocol.store(self, ('randomVals', randomVals),('elgPoints', elgPoints),('A',A), ('a',a),('common_pk',common_pk) )
        Protocol.store(self,('hashedPoints', hashedPoints))
        Protocol.setState(self, 5)
        return {'S3':S3}


    def receiver_transfer4(self, input):
        print("RECEIVER 4")
        g = Protocol.get(self,['g'])[0]
        receiver_sk = Protocol.get(self, ['receiver_sk'])[0] 
        S3 = Protocol.get(self,['S3'])[0]
        A = S3['A']
        receiver_sk_bitstring = bin(int(receiver_sk))
        print("Type of Receiver_sk:", type(receiver_sk_bitstring))
        print("Receiver_sk:", receiver_sk_bitstring)
        bitstring = str(receiver_sk_bitstring) 
        bitstring_list = []
        bitstring_list.extend(bitstring)
        bitstring_list = bitstring_list[2:]
        bitstring  = ''.join(bitstring_list)
        bitstring = bitstring.zfill(256)
        print("sk bitstring: ", bitstring, "len of bitstring:", len(bitstring))
        B = []
        b = []
        for i in  range(256):
            b.append( self.group.random(ZR) )
            B.append( (A[i] ** int(bitstring[i]) ) * (g ** b[i])   )
        #receiver_sk_bytes = objectToBytes(receiver_sk, self.group)
        Protocol.store(self, ('b',b),('B',B),('bitstring',bitstring), ('A',A))
        Protocol.setState(self,6)
        #Protocol.setState(self,None)
        return {'B':B}


    def init_ot5(self, input):
        print("SENDER 5")
        a = Protocol.get(self,['a'])[0]
        A = Protocol.get(self,['A'])[0]
        B = Protocol.get(self,['B'])[0]
        elgPoints = Protocol.get(self,['elgPoints'])[0]
        print("elg Type:", type(elgPoints))
        #enc = {}
        enc = []
        for i in range(256):
            key0 = SHA2(objectToBytes( B[i] ** a[i] ,  self.group))
            key1 = SHA2(objectToBytes( (B[i]* (1/A[i])) ** a[i]   , self.group))
            cipher0 = SymmetricCryptoAbstraction(key0, alg=AES, mode=MODE_CBC)
            cipher1 = SymmetricCryptoAbstraction(key1, alg=AES, mode=MODE_CBC)
            E0 = cipher0.encrypt(objectToBytes(elgPoints[i][0],self.group))
            E1 = cipher1.encrypt(objectToBytes(elgPoints[i][1],self.group))
            if DEBUG:
                if i < 256:
                    print("i: ", i)
                    print("k0: ",key0)
                    print("k1: ",key1)
                    print("E0: ",E0)
                    print("E1: ",E1)
                    print("dec: ",bytesToObject(cipher0.decrypt(E0),self.group))
                    print("\n")
            enc.append([E0, E1])
        Protocol.store(self, ('enc', enc))
        Protocol.setState(self,7)
        return {'enc':enc}
        
        
    def recv_ot6(self, input):
        print("RECEIVER 6")
        common_pk = Protocol.get(self, ['common_pk'])[0]
        enc = Protocol.get(self,['enc'])[0]
        if DEBUG:
            print("enc len: ", len(enc), "\n")
        bitstring = Protocol.get(self,['bitstring'])[0]
        b = Protocol.get(self,['b'])[0]
        B = Protocol.get(self,['B'])[0]
        A = Protocol.get(self,['A'])[0]
        keys = []
        recElgPoints = []
        decPoints = []
        for i in range(256):
            key = SHA2(objectToBytes(A[i]**b[i], self.group))
            keys.append( key ) 
            if i < 3 and DEBUG:
                print("enc: ",enc[i][0])
                print("key: ",key)
                print("\n")
            if bitstring[i] == '0':
                cipher = SymmetricCryptoAbstraction(key, alg=AES, mode=MODE_CBC)
                dec = cipher.decrypt(enc[i][0])
                recElgPoints.append(bytesToObject(dec, self.group))
            else:
                cipher = SymmetricCryptoAbstraction(key, alg=AES, mode=MODE_CBC)
                dec = cipher.decrypt(enc[i][1])
                recElgPoints.append(bytesToObject(dec, self.group))
        print("len recElgPoints", len(recElgPoints)) 
        if DEBUG:
            for i in range(256):
                print("i",i)
                print("bit:", bitstring[i], "type:", type(bitstring[i]))
                print("enc[i]: ", enc[i][0])
        recRandVals, reRandEnc = ReRandElGamal(recElgPoints, common_pk, self.group)
        Protocol.store(self,('recRandVals',recRandVals), ('reRandEnc',reRandEnc))
        Protocol.setState(self,8)
        return {'reRandEnc':reRandEnc}

    def sender_dec_layer7(self, input):
        print("SENDER 7")
        g = Protocol.get(self, ['g'])[0]
        randomVals = Protocol.get(self, ['randomVals'])[0]
        reRandEnc  = Protocol.get(self, ['reRandEnc'])[0]
        common_pk  = Protocol.get(self, ['common_pk'])[0]
        decSenderLayer = []
        if len(reRandEnc) != len(randomVals):
            print("Values and randoms are of different dimension")
            return None
        for i in range(len(randomVals)):
            second_coord = reRandEnc[i]*(1/ (common_pk  ** randomVals[i]))
            decSenderLayer.append(second_coord)
        Protocol.store(self,('decSenderLayer', decSenderLayer))
        Protocol.setState(self,9)
        return {'decSenderLayer':decSenderLayer}

    def final_recv8(self, input):
        print("RECEIVER 8")
        decSenderLayer = Protocol.get(self, ['decSenderLayer'])[0]
        recRandVals    = Protocol.get(self, ['recRandVals'])[0]
        common_pk      = Protocol.get(self, ['common_pk'])[0]
        decRecLayer = []
        if len(recRandVals) != len(decSenderLayer) :
            print("Values and randoms are of different dimension")
            return None
        for i in range(len(recRandVals)):
            second_coord = decSenderLayer[i] * (1/ (common_pk ** recRandVals[i] )   )
            decRecLayer.append(second_coord)
        if DEBUG:
            for i in range(3):
                print(decRecLayer[i])

        Protocol.store(self, ('decRecLayer', decRecLayer))
        msg  = "OK"
        Protocol.setState(self,10)
        return {'msg':msg}

    def send_encryptions9(self,input):
        print("SENDER 9")
        print("number of class files: ", show_jar_classes('HelloWorld0_wm0.jar'))
        '''
        fp0 = open('HelloWorld0_wm0.jar','rb')
        fp1 = open('HelloWorld0_wm1.jar','rb')
        content0 = fp0.read()
        content1 = fp1.read()'''
        hashedPoints = Protocol.get(self,['hashedPoints'])[0]
        row = len(hashedPoints)
        col = len(hashedPoints[0])
        outArray = []
        for i in range(row):
            for j in range(col):
                if j == 0:
                    filename0 = "./files_to_send/Helloworld"+str(i)+"_wm0.jar"
                    fp0 = open(filename0,'rb')
                    content0 = fp0.read()
                    cipher = SymmetricCryptoAbstraction(hashedPoints[i][j], alg=AES, mode=MODE_CBC)
                    enc = cipher.encrypt(content0)
                    outArray.append(enc)
                    fp0.close()
                else:
                    filename1 = "./files_to_send/Helloworld"+str(i)+"_wm1.jar"
                    fp1 = open(filename1,'rb')
                    content1 = fp1.read()
                    cipher = SymmetricCryptoAbstraction(hashedPoints[i][j], alg=AES, mode=MODE_CBC)
                    enc = cipher.encrypt(content1)
                    outArray.append(enc)
                    fp1.close()
        outArray_store = outArray
        Protocol.store(self,('outArray_store',outArray_store))
        #Protocol.store(self,())
        print("outArray[130]", outArray[130])
        outArray1 = outArray[:128]
        print("len outArray", len(outArray))
        Protocol.setState(self,11)
        #Protocol.setState(self,None)
        return {'outArray1':outArray1}

    def recv_encryptions10(self, input):
        print("RECEIVER 10")
        outArray1 = Protocol.get(self,['outArray1'])[0]
        Protocol.store(self,('outArray1',outArray1))
        msg = "GotIt"
        Protocol.setState(self,12)
        return {'msg':msg}

    def send_enc_part2_11(self,input):
        print("SENDER 11")
        outArray = Protocol.get(self, ['outArray_store'])[0]
        print("size outArray",len(outArray) )
        print("outArray[130]",outArray[130])
        outArray2 = outArray[128:256]
        Protocol.setState(self, 13)
        #Protocol.setState(self, None)
        return {'outArray2':outArray2}

    def rec_enc_part2_12(self,input):
        print("RECEIVER 12")
        outArray2 = Protocol.get(self,['outArray2'])[0]
        Protocol.store(self,('outArray2',outArray2))
        msg = "GotIt"
        Protocol.setState(self,14)
        #Protocol.setState(self,None)
        return {'msg':msg}

    def send_enc_part3_13(self,input):
        print("SENDER 13")
        outArray = Protocol.get(self, ['outArray_store'])[0]
        outArray3 = outArray[256:384]
        Protocol.setState(self, 15)
        return {'outArray3':outArray3}

    def rec_enc_part3_14(self,input):
        print("RECEIVER 12")
        outArray3 = Protocol.get(self,['outArray3'])[0]
        Protocol.store(self,('outArray3',outArray3))
        msg = "GotIt"
        Protocol.setState(self,16)
        return {'msg':msg}

    def send_enc_part4_15(self,input):
        print("SENDER 13")
        outArray = Protocol.get(self, ['outArray_store'])[0]
        outArray4 = outArray[384:512]
        Protocol.setState(self, None)
        return {'outArray4':outArray4}

    def rec_enc_part4_16(self,input):
        print("RECEIVER 12")
        outArray4 = Protocol.get(self,['outArray4'])[0]
        Protocol.store(self,('outArray4',outArray4))
        msg = "GotIt"
        outArray1 = Protocol.get(self, ['outArray1'])[0]
        outArray2 = Protocol.get(self, ['outArray2'])[0]
        outArray3 = Protocol.get(self, ['outArray3'])[0]
        outArray4 = Protocol.get(self, ['outArray4'])[0]
        outArray = outArray1 + outArray2 + outArray3 + outArray4
        print("Length of outArray", len(outArray))
        print("Length of outArray[0]", len(outArray[0]))

        bitstring = Protocol.get(self, ['bitstring'])[0]
        print("Length of bitstring:", len(bitstring))
        print("bitstring",bitstring)
        decRecLayer = Protocol.get(self, ['decRecLayer'])[0]
        print("len of decRecLayer", len(decRecLayer))
        hashPoints = []
        for i in range(len(decRecLayer)): 
            hashPoints.append(SHA2(objectToBytes(decRecLayer[i],self.group)))
        print("Length of hashPoints", len(hashPoints))
        decryptedStrings = []
        for i in range(len(hashPoints)):
            if bitstring[i] == '0':
                cipher = SymmetricCryptoAbstraction(hashPoints[i], alg=AES, mode=MODE_CBC)
                dec = cipher.decrypt(outArray[2*i])
                decryptedStrings.append(dec)
            if bitstring[i] == '1':
                cipher = SymmetricCryptoAbstraction(hashPoints[i], alg=AES, mode=MODE_CBC)
                dec = cipher.decrypt(outArray[2*i+1])
                decryptedStrings.append(dec)
        print("len of decrytedStrings array:", len(decryptedStrings))
        
        for i in range(len(decryptedStrings)):
            filename = "./rcvd_files/HelloWorld"+ str(i)+".jar"
            #print(filename)
            fh = open(filename, 'wb+')
            fh.write(decryptedStrings[i])
            fh.close()
        Protocol.setState(self, None)

        return {'msg':msg}


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: %s [-r or -s]" % sys.argv[0])
        exit(-1)

    if sys.argv[1] == "-r":
        print("Operating as receiver...")
        svr = socket(AF_INET, SOCK_STREAM)
        svr.bind((HOST, PORT))
        svr.listen(1)
        svr_sock, addr = svr.accept()
        print("Connected by ", addr)
        msgs = None
        _name, _type, _sock = "receiver", RECEIVER, svr_sock
#       sp.setup( {'name':"receiver", 'type':_type, 'socket':svr_sock} )
    elif sys.argv[1] == "-s":
        print("Operating as sender...")
        clt = socket(AF_INET, SOCK_STREAM)
        clt.connect((HOST, PORT))
        clt.settimeout(15)
        msgs = ['one', 'two']
        _name, _type, _sock = "sender", SENDER, clt
    else:
        print("Usage: %s -r or -s" % sys.argv[0])
        exit(-1)

#    group = PairingGroup('library/a.param')
    sp = SoftwareTransfer(msgs)
    sp.setup( {'name':_name, 'type':_type, 'socket':_sock} )
    # run as a thread...
    sp.execute(_type)


