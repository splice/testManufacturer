package main

import (
	"flag"
	"fmt"
	"io/ioutil"
	"log"
	"os"
	"os/user"
	"path/filepath"
	"runtime"

	"github.com/splice/rto"
)

var (
	flagPluginName = flag.String("plugin", "testPlugin", "the name of the plugin the license is generated for")
	sharedKey = []byte{0x1a, 0xe9, 0x85, 0xf6, 0xef, 0xda, 0xc7, 0xd6, 0x7b, 0x23, 0xda, 0x5, 0x49, 0x16, 0x9e, 0x63, 0xcb, 0x37, 0xd6, 0xe9, 0x1f, 0x2, 0x8d, 0x78, 0x3e, 0x44, 0xee, 0x92, 0x3c, 0x7, 0x7e, 0x55, 0xfa, 0x2e, 0xc8, 0x63, 0x90, 0xca, 0xe5, 0x46, 0x27, 0x12}
)

func main() {
	flag.Parse()
	rto.Debug = false

	if *flagPluginName == "" {
		fmt.Println("you need to give the plugin to look a lease license for")
		os.Exit(1)
	}
	licPath := filepath.Join(appFolderPath(), fmt.Sprintf("%s.lic", *flagPluginName))
	if _, err := os.Stat(licPath); os.IsNotExist(err) {
		fmt.Println("no lease license file available at", licPath)
	}
	f, err := os.Open(licPath)
	if err != nil {
		fmt.Println("failed to open", licPath)
		os.Exit(1)
	}
	defer f.Close()
	data, err := ioutil.ReadAll(f)
	if err != nil {
		fmt.Println("failed to read license content", err)
		os.Exit(1)
	}
	lease, err := rto.DecodeLease(data, sharedKey)
	if err != nil {
		fmt.Println("failed to decode lease file:", err)
		os.Exit(1)
	}
	fmt.Println(lease)
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
