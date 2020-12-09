# About
A simple fan controller for some Dell laptops. Works around the annoying behaviour where the fans turn continuously on/off when managed by the BIOS/kernel.

# WARNING - USE AT YOUR OWN RISK
Overrides BIOS fan control on an M4600 and has unknown behavior on other machines. **In the worst case, may brick/destroy your system**. Tested on a Dell M4600 (Arch Linux, kernel 5.9.13-arch1-1, Intel i7-2760QM, NVIDIA Quadro 2000M). May work on Latitude E6420/E6520/E6430/E6530/E6440/E6540/..., see [clopez/dellfan](https://github.com/clopez/dellfan).

# Known issues
Disabling BIOS fan control also disables brightness controls.

# Installation
A systemd service provided for your convenience. I strongly recommend running the program once to see it works before installing.

Only do this if you know what you're doing.

`cp dellfancontrol /usr/bin/`

`cp dellfancontrol.service /etc/systemd/system/`

and reboot. To start at boot, do `systemctl enable dellfancontrol.service`.
