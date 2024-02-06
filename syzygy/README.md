# Syzygy tables

Download 5-man tables:

```bash
cd Goldfish/syzygy
bash download.sh  # Will take a little while.
```

Verify download:

```bash
md5sum -c 5-man-checksums.md5 | grep OK | wc -l
cat links.txt | wc -l
```

If both commands output the same number (290), then all is OK.
