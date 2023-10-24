package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/user"
	"path/filepath"
	"runtime"

	// TODO: Only uncomment if the provider requires a custom payload.
	// "github.com/splice/testmanufacturer"
	"github.com/splice/rto"
)

var (
	flagPluginName  = flag.String("plugin", "testPlugin", "the name of the plugin the license is generated for")
	flagSerial      = flag.String("serial", "", "the serial license for the lease to be used with the plugin's authorization procedure")
	flagLeaseStatus = flag.Int("status", 1, "lease status <0 issues, 0: unknown, >0: active")
	flagSignature   = flag.Bool("sign", true, "whether or not the license should be cryptographically signed")
	flagPrivateKey  = flag.String("private-key", "", "the private key for encryption")

	sharedKey = []byte{0xa6, 0x62, 0x58, 0x56, 0x22, 0x5b, 0xde, 0xfb, 0xc5, 0xc3, 0x9, 0x3a, 0x3a, 0xd9, 0x45, 0xda, 0xd6, 0x7e, 0x74, 0x4d, 0x6, 0x5c, 0xbf, 0x5, 0x7f, 0x89, 0xdc, 0x45, 0xb6, 0x28, 0x96, 0x94, 0xc9, 0x58, 0xac, 0xf1, 0xa2, 0x85, 0x30, 0x41, 0x9d, 0x76}
)

func main() {
	flag.Parse()
	privateKey := *flagPrivateKey
	if privateKey == "" {
		privateKey = "B2j8yECD5DVltV5jkKCrT49+bN3KnbYvmnjkIIjZmfzJ1ppztVAwTu3BcdZT74b1sYVfyZ7vj2vW7g6RrFmavQ=="
	}

	rto.Debug = false
	args := &rto.LeaseArgs{
		PluginName: *flagPluginName,
		State:      rto.LeaseState(*flagLeaseStatus),
		UserID:     42,
		Username:   "matt",
		LicenseKey: []byte(*flagSerial),
		Sign:       *flagSignature,
		PrivateKey: privateKey,
	}

	// TODO: If the provider requires a custom payload, uncomment this and import
	// the provider's package.
	//
	// You will need to re-compile this file. You can run `make recompile` for that.
	// if *flagSerial == "" {
	//   fmt.Printf("Serial not provided. Creating custom payload.\n")
	//   args.LicenseKey := testmanufacturer.GeneratePayload(args)
	// }

	leaseContent := rto.NewLeaseLicense(args)

	appFolder := appFolderPath()
	// in case the splice folder wasn't there
	if err := os.MkdirAll(appFolder, os.ModePerm); err != nil {
		panic(err)
	}
	licPath := filepath.Join(appFolder, fmt.Sprintf("%s.lic", *flagPluginName))
	f, err := os.Create(licPath)
	if err != nil {
		fmt.Println("failed to create the license file at", licPath)
		os.Exit(1)
	}
	defer f.Close()
	if rto.Debug {
		fmt.Printf("Shared key: %#v\n", sharedKey)
	}
	licContent, err := leaseContent.Encode(sharedKey)
	if err != nil {
		fmt.Println("failed to generate license content:", err)
		os.Exit(1)
	}
	if _, err = f.Write(licContent); err != nil {
		fmt.Println("failed to write the license", err)
		os.Exit(1)
	}

	fmt.Println(leaseContent)
	fmt.Println("saved to disk at:", licPath)
}

func homeDir() string {
	usr, err := user.Current()
	if err != nil {
		log.Fatal(err)
	}
	return usr.HomeDir
}

func appFolderPath() string {
	if runtime.GOOS == "windows" {
		return filepath.Join(homeDir(), "AppData", "Local", "SpliceSettings", "license")
	}
	return filepath.Join(homeDir(), "Library", "Audio", "Plug-Ins", "Support", "Splice", "license")
}
