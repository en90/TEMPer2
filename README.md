# TEMPer2-getdegs

GNU/Linux utility for reading temperature measurments
using RDing TEMPer2 USB device.

## Building `getdegs` and adding udev rule for TEMPer2

```bash
make && \
sudo make rules-install
```

In case the TEMPer2 was already plugged in,
physically unplug and plug it back in.

## Usage

```bash
./getdegs [-n <count>] [--sleep <num>]
```
