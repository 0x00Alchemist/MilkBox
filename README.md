# MilkBox
**MilkBox** - PoC of dumping EFI runtime drivers.

## Demo
You can watch it [here](https://youtu.be/XoXePiKZJYY).

## Usage
```
rtd - Locate runtime drivers (should be performed firstly)
wd - Write dump to binary file, dump location - "C:\MilkBox\"
ud - Uninstall driver
ex - Exit from program
```

## Compilation
The **MilkBox** driver is compiled by any **WDK** designed for Windows 10 and above. The client is compiled with **MSVC** v143 or higher.

## Restrictions
Since the driver is test signed only, you will need to **disable DSE** (Driver Signature Enforcement) while the driver is in use.
**PoC was only tested on a virtual machine**. Although theoretically everything should be fine, but be careful if you use the driver on a physical machine.

## Acknowledgments
[Alex Ionescu](http://publications.alex-ionescu.com/Recon/ReconBru%202017%20-%20Getting%20Physical%20with%20USB%20Type-C,%20Windows%2010%20RAM%20Forensics%20and%20UEFI%20Attacks.pdf), [Satoshi Tandasat](https://standa-note.blogspot.com/2020/12/experiment-in-extracting-runtime.html) (for some tricks with physical memory which I implemented too).

## Credits
[0x00Alchemist](https://github.com/0x00Alchemist) (2023)