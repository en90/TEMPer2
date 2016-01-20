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

### Example

```bash
./getdegs -n 5 --sleep 10
```

```
Device already detached
Device already detached
Time [s]	Internal [°C]	External [°C]
1453332865	       +33.44	       +15.75
1453332875	       +33.44	       +15.81
1453332885	       +33.38	       +15.75
1453332895	       +33.31	       +15.75
1453332905	       +33.31	       +15.75
```
