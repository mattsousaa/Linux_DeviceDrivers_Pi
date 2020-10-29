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

**Tip:** Browse the source code of linux/ online in [woboq](https://code.woboq.org/linux/). Here on this site you can find the linux files needed to understand the codes instead of looking for them on your console.

## 002_pseud_char_driver - How to test this driver?

In this character driver we will give support to handle the below user level system calls:
* open
* close
* read
* write
* llseek

First, you can test running some Linux commands. Compile the code by running **make** and observe the generated **pcd.ko** module. Load it with commands below.

```
$ sudo -s
$ insmod pcd.ko
$ dmesg | tail
```

After **dmesg** command you will see the module was successfully created. The commands below will be something close to what you will see in your terminal.

```
[ 6540.927391] pcd_driver_init : Device number <major>:<minor> = 235:0
[ 6540.927880] pcd_driver_init : Module init was successful
```

Use **echo** command to send some data to our device:

```
$ echo "Hello I'm just testing this driver" > /dev/pcd
```

Type **dmesg** command to see the kernel logs below:

```
[ 6967.419979] pcd_open : Open was successful (echo issued open system call/opened our device)
[ 6967.420005] pcd_write : Write requested for 35 bytes (echo requested 35 bytes)
[ 6967.420007] pcd_write : Current file position = 0 (whenever open happens the file position is always 0)
[ 6967.420008] pcd_write : Number of bytes successfully written = 35 
[ 6967.420009] pcd_write : Updated file position = 35 (pcd_write returns 35)
[ 6967.420015] pcd_release : Release was successful (echo issued a close system call/device was closed)  
```

Now, the message is stored in our device memory. You can read by typing **cat** command to read our device buffer.

```
$ cat /dev/pcd
Hello I'm just testing this driver
```
Check the kernel log typing **dmesg** command. The result is similar to this:

```
[ 8238.554876] pcd_open : Open was successful (cat command opened our device)
[ 8238.554885] pcd_read : Read requested for 131072 bytes (cat tries to read this much amount of bytes. Why? Because that's how cat application is written)
[ 8238.554886] pcd_read : Current file position = 0 
[ 8238.554888] pcd_read : Number of bytes successfully read = 512 (our device memory size has just 512 bytes)
[ 8238.554888] pcd_read : Updated file position = 512 (pcd_read returns 512) 
[ 8238.554898] pcd_read : Read requested for 131072 bytes (cat command is not happy because it requested for these many amount of bytes. So cat again requested for read)
[ 8238.554899] pcd_read : Current file position = 512 (there is nothing to read)
[ 8238.554899] pcd_read : Number of bytes successfully read = 0 (that's why our drive returned zero (EOF- End of file))
[ 8238.554900] pcd_read : Updated file position = 512 
[ 8238.554907] pcd_release : Release was successful (cat issued a close system call/device was closed)
```
Create a new file in any directory and put some text inside of it. Let's say that your file has 1567 bytes. Let's try to copy its content to our device memory typing the following command: 

```
$ cp file /dev/pcd
cp: error writing '/dev/pcd': Cannot allocate memory
```

Do you see that error? What happended? Let's analyse by **dmesg | tail** command. 

```
[10580.529511] pcd_open : Open was successful (cp command opened our device)
[10580.529525] pcd_write : Write requested for 1567 bytes (write requested for 1679 bytes, that was the size of the file)
[10580.529526] pcd_write : Current file position = 0 
[10580.529526] pcd_write : Number of bytes successfully written = 512 
[10580.529527] pcd_write : Updated file position = 512 
[10580.529528] pcd_write : Write requested for 1055 bytes (cp command is not happy and it again issued another write command)
[10580.529529] pcd_write : Current file position = 512 (there is no space to write more data. That's why our driver returned ENOMEM (out of memory)) 
[10580.529529] pcd_write : No space left on the device 
[10580.529692] pcd_release : Release was successful
```

Copy command decode that error code and it printed cannot allocate memory. Finally, remove the kernel module.
```
$ rmmod pcd.ko
```

```
[10855.187923] pcd_driver_cleanup : Module unloaded
```

