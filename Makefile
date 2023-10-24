default: recompile

recompile: darwin win

darwin:
	mkdir -p doc/tools/darwin
	GOOS=darwin go build -o doc/tools/darwin/generate -ldflags "-s -w" ./cmd/generate
	GOOS=darwin go build -o doc/tools/darwin/decode -ldflags "-s -w" ./cmd/decode

win:
	mkdir -p doc/tools/windows
	GOOS=windows go build -o doc/tools/windows/generate.exe -ldflags "-s -w" ./cmd/generate
	GOOS=windows go build -o doc/tools/windows/decode.exe -ldflags "-s -w" ./cmd/decode
