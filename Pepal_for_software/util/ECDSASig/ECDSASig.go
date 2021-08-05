package main

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"
	"crypto/x509"
	"encoding/base64"
	"errors"
	"flag"
	"fmt"
	"io"
	"math/big"
	"strings"
)

var curve = elliptic.P256()

func makePrivateKey(privateKeyD string, publicKeyB64 string) (*ecdsa.PrivateKey, error) {
	publicKeyBytes, err := base64.URLEncoding.DecodeString(publicKeyB64)
	d := big.NewInt(0)
	d, valid := d.SetString(privateKeyD, 0)
	if !valid || err != nil {
		return &ecdsa.PrivateKey{}, errors.New("[-] Unable to parse private key D")
	}
	publicKey, err := x509.ParsePKIXPublicKey(publicKeyBytes)
	switch publicKey := publicKey.(type) {
	case *ecdsa.PublicKey:
		res := ecdsa.PrivateKey{D: d, PublicKey: *publicKey}
		return &res, nil
	default:
		return &ecdsa.PrivateKey{}, errors.New("[-] Unable to parse public key")
	}
}

func generateKeys(random io.Reader) (string, string) {
	privateKey, _ := ecdsa.GenerateKey(curve, random)
	publicKeyBytes, _ := x509.MarshalPKIXPublicKey(&privateKey.PublicKey)
	pkB64 := base64.URLEncoding.EncodeToString(publicKeyBytes)
	skD := privateKey.D.String()
	return pkB64, skD
}

func sign(message string, privateKeyD string, publicKeyB64 string) (rStr, sStr string, err error) {
	privateKey, err := makePrivateKey(privateKeyD, publicKeyB64)
	r, s, err := ecdsa.Sign(rand.Reader, privateKey, []byte(message))
	if err != nil {
		fmt.Printf("unable to sign %s", err.Error())
		rStr = big.NewInt(0).String()
		sStr = rStr
	}
	rStr = r.String()
	sStr = s.String()
	return rStr, sStr, err
}

func verify(message string, publicKeyB64 string, rStr string, sStr string) bool {
	publicKeyBytes, err := base64.URLEncoding.DecodeString(publicKeyB64)
	publicKey, err := x509.ParsePKIXPublicKey(publicKeyBytes)
	if err != nil {
		return false
	}
	r := new(big.Int)
	r.SetString(rStr, 0)
	s := new(big.Int)
	s.SetString(sStr, 0)

	switch publicKey := publicKey.(type) {
	case *ecdsa.PublicKey:
		return ecdsa.Verify(publicKey, []byte(message), r, s)
	default:
		return false
	}
}
func verifyMulti(message string, publicKeys []string, signatures []string) bool {
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

func main() {
	mode := flag.String("mode", "generate", "Generate Public and private key for signing[generate, sign, verify]")
	privateKeyD := flag.String("privateKeyD", "", "Private key to sign message")
	publicKey := flag.String("publicKey", "", "Public Key to verify message")
	rs := flag.String("rs", "", "r,s from ECDSA signature")
	message := flag.String("message", "example message", "message to be signed/verified using privateKey/publicKey")

	flag.Parse()

	if *mode == "generate" {
		fmt.Println("Generating public and secret key pairs for APDL protocol...")
		public, signing := generateKeys(rand.Reader)
		fmt.Println("Public Key: " + public)
		fmt.Println("Signing Key (D): " + signing)
	} else if *mode == "sign" {
		rStr, sStr, err := sign(*message, *privateKeyD, *publicKey)
		if err != nil {
			fmt.Println(err)
		}
		fmt.Println("r: " + rStr)
		fmt.Println("s: " + sStr)
		fmt.Println("r,s: " + rStr + "," + sStr)
	} else if *mode == "verify" {
		temp := strings.Split(*rs, ",")
		if len(temp) < 2 {
			fmt.Println("[-] invalid signature passed")
		} else if len(*message) == 0 || len(*publicKey) == 0 {
			fmt.Println("[-] invalid public key passed")
		}
		r := temp[0]
		s := temp[1]
		result := verify(*message, *publicKey, r, s)
		if result {
			fmt.Println("Verified!")
		} else {
			fmt.Println("Invalid signature/key")
		}
	} else if *mode == "test1" {
		fmt.Println("[+] Testing signatures...")
		m := "some message"
		public, signing := generateKeys(rand.Reader)
		fmt.Println("Public Key: " + public)
		fmt.Println("Signing Key: " + signing)
		rStr, sStr, _ := sign(m, signing, public)
		fmt.Println("r: " + rStr)
		fmt.Println("s: " + sStr)
		fmt.Println("Valid verification: ")
		fmt.Println(verify(m, public, rStr, sStr))
		fmt.Println("Invalid verification: ")
		fmt.Println(verify("some other string", public, rStr, sStr))
		fmt.Println("Invalid verification: ")
		fmt.Println(verify(m, public, big.NewInt(298075).String(), sStr))
	} else if *mode == "test2" {
		m := "some other message"
		public1, signing1 := generateKeys(rand.Reader)
		public2, signing2 := generateKeys(rand.Reader)
		rStr1, sStr1, _ := sign(m, signing1, public1)
		rStr2, sStr2, _ := sign(m, signing2, public2)
		fmt.Println("Valid Multi verification: ")
		pks := []string{public1, public2}
		sigs := []string{rStr1 + "," + sStr1, rStr2 + "," + sStr2}
		fmt.Println(verifyMulti(m, pks, sigs))
	} else if *mode == "test3" {
		fmt.Println("[+] Private key comparison...")
		m := "some other message"
		public1, signing1 := generateKeys(rand.Reader)
		signing2, _ := makePrivateKey(signing1, public1)
		rStr1, sStr1, _ := sign(m, signing1, public1)
		rStr2, sStr2, _ := sign(m, signing2.D.String(), public1)
		pks := []string{public1, public1}
		sigs := []string{rStr1 + "," + sStr1, rStr2 + "," + sStr2}
		fmt.Println(verifyMulti(m, pks, sigs))
	} else {
		fmt.Println("[-] Invalid mode")
	}
}
