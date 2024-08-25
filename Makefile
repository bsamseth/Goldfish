EXE ?= goldfish

all:
	cargo build --release --bin goldfish
	cp -v target/release/goldfish $(EXE)
