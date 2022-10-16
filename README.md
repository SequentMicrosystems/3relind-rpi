[![3relind-rpi](res/sequent.jpg)](https://sequentmicrosystems.com)
# 3relind-rpi
Drivers for 3-Relays card

## Usage

```bash
~$ git clone https://github.com/SequentMicrosystems/3relind-rpi.git
~$ cd 3relind-rpi/
~/3relind-rpi$ ./build_install_all.sh
```

Now you can access all the functions of the relays board through the command "3relind". Use -h option for help:
```bash
~$ 3relind -h
```

If you clone the repository, any update can be made with the following commands:

```bash
~$ cd 3relind-rpi/  
~/3relind-rpi$ git pull
~/3relind-rpi$ ./build_install_all.sh
```  

You can clean and uninstall with the following commands:

```bash
~$ cd 3relind-rpi/
~/3relind-rpi$ ./clean_uninstall_all.sh
```