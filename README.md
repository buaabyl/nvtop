# nVidia GPU top

- TODO1: write htop like interface
- TODO2: support intel and AMD.

I got an dGPU:)

for intel graphic card, just this tool/library:

```
$ sudo apt-get install intel-gpu-tools
```


I write this script to monitor nVidia GPU,

```
# install nvidia X11-extend control library
$ sudo apt-get install libxnvctrl-dev

# build nvtop
$ make 

$ ./nvtop
Display:1
  VideoBIOS      : 86.08.11.00.2b
  Driver version : 390.48
  NV-CONTROL X extension present
  version        : 1.29

  Screen:0
    product name  : GeForce MX150
    vbios version : 86.08.11.00.2b
    driver version: 390.48
    gpu total memory: 2.000 Gb
    gpu utilization (0-100): graphics=0, memory=0, PCIe=0
```

