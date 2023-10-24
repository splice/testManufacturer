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

	sharedKey = []byte{0x1a, 0xe9, 0x85, 0xf6, 0xef, 0xda, 0xc7, 0xd6, 0x7b, 0x23, 0xda, 0x5, 0x49, 0x16, 0x9e, 0x63, 0xcb, 0x37, 0xd6, 0xe9, 0x1f, 0x2, 0x8d, 0x78, 0x3e, 0x44, 0xee, 0x92, 0x3c, 0x7, 0x7e, 0x55, 0xfa, 0x2e, 0xc8, 0x63, 0x90, 0xca, 0xe5, 0x46, 0x27, 0x12}
)

func main() {
	flag.Parse()
	privateKey := *flagPrivateKey
	if privateKey == "" {
		privateKey = "yMysCWyQdFzeNuhYYZ/7eB6BxbuxsIgA6orGwguov2gDRAvg5v5x3M86qg6zEpTWCVxtmGHiEGppxtlFf2huWQ=="
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
