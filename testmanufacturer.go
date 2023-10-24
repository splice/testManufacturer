package testmanufacturer

import (
	"fmt"

	"github.com/splice/rto"
)

var (
	// SharedKey is the crypto key shared between the plugin provider and splice to encrypt the data.
	SharedKey = []byte{0x1a, 0xe9, 0x85, 0xf6, 0xef, 0xda, 0xc7, 0xd6, 0x7b, 0x23, 0xda, 0x5, 0x49, 0x16, 0x9e, 0x63, 0xcb, 0x37, 0xd6, 0xe9, 0x1f, 0x2, 0x8d, 0x78, 0x3e, 0x44, 0xee, 0x92, 0x3c, 0x7, 0x7e, 0x55, 0xfa, 0x2e, 0xc8, 0x63, 0x90, 0xca, 0xe5, 0x46, 0x27, 0x12}
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
