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
	sharedKey = []byte{0xa6, 0x62, 0x58, 0x56, 0x22, 0x5b, 0xde, 0xfb, 0xc5, 0xc3, 0x9, 0x3a, 0x3a, 0xd9, 0x45, 0xda, 0xd6, 0x7e, 0x74, 0x4d, 0x6, 0x5c, 0xbf, 0x5, 0x7f, 0x89, 0xdc, 0x45, 0xb6, 0x28, 0x96, 0x94, 0xc9, 0x58, 0xac, 0xf1, 0xa2, 0x85, 0x30, 0x41, 0x9d, 0x76}
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
