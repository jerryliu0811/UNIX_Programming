# WHAT IS THIS:
    It's a simple chat program in LAN using Linux socket,  
    and it can broadcast chat messages using layer 2 broadcast packets.  

# HOW TO COMPILE/REMOVE EXECUTABLE CODE:
    1. use 'make' to compile the executable code.
    2. use 'make clean' to remove the executable code.

# HOW TO RUN THIS PROGROM:
    1. use 'sudo ./lanchat.o'

# FUNCTIONALITY THAT THIS CHAT PROGRAM SUPPORT:
    1. Enumerate all Ethernet compatible network interfaces.
    2. Ask the user to provide his/her username.
    3. Repeatedly ask the user to enter his/her message.    
       The message is then broadcasted to all enumerated Ethernet-compatible network interfaces.
    4. At the same time, this program would receive messages broadcasted by other host in connected LANs.

# RUNNING EXAMPLE:
  +  $ sudo ./lanchat
```
    Enumerated network interfaces:
    2 - enp0s3     010.000.002.015 0xffffff00 (010.000.002.255) 08:00:27:98:ca:e5
    4 - enp0s8     010.000.001.001 0xffffff00 (010.000.001.255) 08:00:27:6a:11:25
    Enter your name: aaaaa
    Welcome, 'aaaaa'!
    >>> <08:00:27:5b:d1:4c> [bbbbb]: hello, everybody here?
    hello!
    >>> <08:00:27:5b:d1:4c> [bbbbb]: this is bbbbb

    >>>
    >>>
```
