package main

import (
	"crypto/ecdsa"
	"crypto/x509"
	"encoding/base64"
	"encoding/json"
	"fmt"
	"math/big"
	"strconv"
	"strings"
	"time"

	"github.com/hyperledger/fabric/core/chaincode/shim"
	"github.com/hyperledger/fabric/protos/peer"
)

var StartAmount = 0                /// number of violations incurred
var ContractKeyPrefix = "contract" // + sender pk + recipient pk
var SenderIP = "0.0.0.0"
var SenderPort = "0"

type SimpleAsset struct {
}

type Party struct {
	pubKey    string
	IPAddress string
	Port      string
}

type APDL struct {
	Status         string
	SoftwareOwner  Party
	SoftwareUser   Party
	ContractExpiry time.Time
	DepositAmount  int
}

// Init called with the intention to create a new NDA contract
func (t *SimpleAsset) Init(stub shim.ChaincodeStubInterface) peer.Response {
	return shim.Success([]byte("[+] Init completed\n"))
}

// Invoke is called per transaction on the chaincode.
func (t *SimpleAsset) Invoke(stub shim.ChaincodeStubInterface) peer.Response {
	// Extract the function and args from the transaction proposal
	fn, args := stub.GetFunctionAndParameters()

	var result string
	var err error
	result = fn
	if fn == "download_request" {
		args := stub.GetStringArgs()
		// args = [sender_pk, recipient_pk, amount, deposit_time, recipient_ip, recipient_port]
		if len(args) != 6 {
			return shim.Error("Incorrect arguments. Expecting [sender_pk, recipient_pk, deposit_amount, deposit_time, recipient_ip, recipient_port]")
		}
		amount, err := strconv.Atoi(args[2])
		if err != nil || amount < 0 {
			return shim.Error("Invalid amount passed")
		}
		dt, err := time.Parse("01/02/2006", args[3])
		if err != nil || !dt.After(time.Now()) {
			return shim.Error("Invalid time passed")
		}
		senderPublicKey := args[0]
		recipientPublicKey := args[1]
		contractKey := ContractKeyPrefix + senderPublicKey + recipientPublicKey
		ds := Party{pubKey: senderPublicKey, IPAddress: SenderIP, Port: SenderPort}
		dr := Party{pubKey: recipientPublicKey, IPAddress: args[4], Port: args[5]}
		apdl := APDL{"download_requested", ds, dr, dt, amount}
		initialAmount := []byte(strconv.Itoa(StartAmount))
		err = stub.PutState(senderPublicKey, initialAmount)
		err = stub.PutState(recipientPublicKey, initialAmount)
		b, err := json.Marshal(apdl)
		err = stub.PutState(contractKey, b)
		if err != nil {
			return shim.Error("[-] Failed to process download request transaction. Error!")
		}
	} else if fn == "penalty" {
		if len(args) != 5 {
			return shim.Error("need to pass recipient sig")
		}
		// args = [message, recipientSig, senderSig, s_pubKey, r_pubKey]
		// Xsig = "rStr,sStr"
		contractKey := ContractKeyPrefix + args[3] + args[4]
		contract, err := stub.GetState(contractKey)
		if err != nil {
			return shim.Error("[-] Failed to get contract")
		}
		var apdl APDL
		json.Unmarshal(contract, &apdl)
		// check if status is active to prevent double penalty.
		publicKeys := []string{apdl.SoftwareOwner.pubKey, apdl.SoftwareUser.pubKey}
		signatures := []string{args[1], args[2]}
		if verifySig(args[0], publicKeys, signatures) != true {
			return shim.Error("[-] Signature verification failed. Penalty not applied.")
		}
		value, err := stub.GetState(apdl.SoftwareUser.pubKey)
		balance, _ := strconv.Atoi(string(value))
		balance = balance + apdl.DepositAmount
		endBalance := []byte(strconv.Itoa(balance))
		stub.PutState(apdl.SoftwareUser.pubKey, endBalance)
		apdl.Status = "penalized"
		b, err := json.Marshal(apdl)
		stub.PutState(contractKey, b)
		result = "[+] Penalty applied\n"

	} else if fn == "refund" {
		if len(args) != 5 {
			return shim.Error("need to pass recipient sig")
		}
		// args = [message, recipientSig, senderSig, r_pubKey, s_pubKey]
		contractKey := ContractKeyPrefix + args[3] + args[4]
		contract, err := stub.GetState(contractKey)
		if err != nil {
			return shim.Error(err.Error())
		}
		var apdl APDL
		json.Unmarshal(contract, &apdl)
		if apdl.Status != "download_request" {
			return shim.Error("[-] Invalid status for refund. Current status: " + apdl.Status)
		}
		tnow := time.Now()
		if tnow.After(apdl.ContractExpiry) {
			apdl.Status = "expired"
			b, _ := json.Marshal(apdl)
			stub.PutState(ContractKey, b)
			result = "[+] APDL contract expired."
		} else {
			return shim.Error("[-] APDL contract not yet expired")
		}
	}
	if err != nil {
		return shim.Error(err.Error())
	}
	// Return the result as success payload
	return shim.Success([]byte(result))
}

// message, apdl.SoftwareOwner.pubKey, apdl.SoftwareUser.pubKey, recipientSig, senderSig
// Xsig = "rStr,sStr"
func verifySig(message string, publicKeys []string, signatures []string) bool {
	verified := false
	if len(publicKeys) != len(signatures) {
		return false
	}

	for i := 0; i < len(publicKeys); i++ {
		publicKeyBytes, err := base64.URLEncoding.DecodeString(publicKeys[i])
		publicKey, err := x509.ParsePKIXPublicKey(publicKeyBytes)
		if err != nil {
			return false
		}
		sig := strings.Split(signatures[i], ",")
		if len(sig) < 2 {
			return false
		}
		r := new(big.Int)
		r.SetString(sig[0], 0)
		s := new(big.Int)
		s.SetString(sig[1], 0)
		switch publicKey := publicKey.(type) {
		case *ecdsa.PublicKey:
			verified = ecdsa.Verify(publicKey, []byte(message), r, s)
		default:
			return false
		}
	}
	return verified
}

// main function starts up the chaincode in the container during instantiate
func main() {
	if err := shim.Start(new(SimpleAsset)); err != nil {
		fmt.Printf("Error starting SimpleAsset chaincode: %s", err)
	}
}
