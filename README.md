## Headless Raspberry Pi Setup

* Create a file with no extension on the SD Card boot partition called **ssh**.
* Create another file called **wpa_supplicant.conf** and again save this on boot partition. In this file, add your network name and password in the asterisk fields as shown below.
* These steps will allow Raspberry to connect to the Wi-Fi network and allow the user to connect via SSH.
```C
ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
update_config=1
country=BR
network={
    ssid="Hotspot_network"
    psk="your_password"
}
```
