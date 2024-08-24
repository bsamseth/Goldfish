EXE ?= goldfish

all:
	cargo build --profile release-lto --bin goldfish
	cp -v target/release-lto/goldfish $(EXE)
