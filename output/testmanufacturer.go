package testmanufacturer

import (
	"fmt"

	"github.com/splice/rto"
)

var (
	// SharedKey is the crypto key shared between the plugin provider and splice to encrypt the data.
	SharedKey = []byte{0xa6, 0x62, 0x58, 0x56, 0x22, 0x5b, 0xde, 0xfb, 0xc5, 0xc3, 0x9, 0x3a, 0x3a, 0xd9, 0x45, 0xda, 0xd6, 0x7e, 0x74, 0x4d, 0x6, 0x5c, 0xbf, 0x5, 0x7f, 0x89, 0xdc, 0x45, 0xb6, 0x28, 0x96, 0x94, 0xc9, 0x58, 0xac, 0xf1, 0xa2, 0x85, 0x30, 0x41, 0x9d, 0x76}
	// PluginName is the name of the plugin
	PluginName = "testPlugin"
)

// NewLeaseLicense returns a new Lease for testManufacturer.
func NewLeaseLicense(args *rto.LeaseArgs) *rto.Lease {
	if args.PluginName == "" {
		args.PluginName = PluginName
	}

	// TODO: If this provider requires a custom payload, uncomment this and change
	// the generatePayload function below.
	// if len(args.LicenseKey) == 0 {
	//   args.LicenseKey = GeneratePayload(args)
	// }

	return rto.NewLeaseLicense(args)
}

// TODO: Add any custom payload here. We can use the default payload most of the
// time.
//
// If you change this function, you'll need to change the `cmd/generate/main.go`
// file too.
// func GeneratePayload(l *rto.LeaseArgs) []byte {
//   return rto.GeneratePayload(l)
// }
