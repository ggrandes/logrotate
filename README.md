# logrotate

Log Rotate based on filesize with Timestamp of lines, open source C code (Apache License, Version 2.0). Do not require any external code/lib.

---

## How works:

Read from stdin and output lines with timestamp in an output file (pathname), rotating them when file size exceeds specified size in bytes.

## Compile (handmade)

    gcc -Wall -O2 logrotate.c -o logrotate

## Running

    ./process-to-log | ./logrotate <pathname> <size-limit-bytes>

## Example output file:

    2013-06-02 00:47:51 > Lorem ipsum dolor sit amet, consectetur adipiscing elit.
    2013-06-02 00:47:51 > Pellentesque in mi ac tellus porttitor condimentum.

---
Inspired in [rotatelogs](http://httpd.apache.org/docs/2.2/programs/rotatelogs.html) and [cronolog](http://cronolog.org/), this is C-minimalistic version.
