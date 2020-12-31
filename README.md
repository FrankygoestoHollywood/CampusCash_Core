CampusCash [CCASH] 2020
===========================================================================================

http://campuscash.org/

What is the CampusCash [CCASH] Blockchain?
-----------------------------------------
*TODO: Update documentation regarding implemented tech as this section is out of date and much progress and upgrades have been made to mentioned sections...*

### Overview
CampusCash is a blockchain project with the goal of creating a blockchain for the student, one where your degrees are stored on the blockchain and can be validated by employers by the pubkey that sent you the degree.

### Blockchain Technology
The CampusCash [CCASH] Blockchain is an experimental smart contract platform that enables 
instant payments to anyone, anywhere in the world in a private, secure manner. 
CampusCash [CCASH] uses peer-to-peer blockchain technology developed by CryptoCoderz and SaltineChips to operate
with no central authority: managing transactions, execution of contracts, and 
issuing money are carried out collectively by the network.

### Custom Difficulty Retarget Algorithm “VRX”
VRX is designed from the ground up to integrate properly with the Velocity parameter enforcement system to ensure users no longer receive orphan blocks.

### Velocity Block Constraint System
Ensuring CampusCash stays as secure and robust as possible, we have implemented what's known as the Velocity block constraint system (developed by CryptoCoderz & SaltineChips). This system acts as a third and final check for both mined and peer-accepted blocks, ensuring that all parameters are strictly enforced.

### Wish (bmw512) Proof-of-Work Algorithm
Wish or bmw512 hashing algorithm is utilized for the Proof-of-Work function and also replaces much of the underlying codebase hashing functions as well that normally are SHA256. By doing so this codebase is able to be both exponentially lighter and more secure in comparison to reference implementations.

Specifications and General info
------------------
CampusCash uses 

	libsecp256k1,
	libgmp,
	Boost1.68, OR Boost1.58,  
	Openssl1.02r,
	Berkeley DB 6.2.32,
	QT5.12.1,
	to compile


General Specs

	Block Spacing: 1 Minute +- 30 seconds
	Stake Minimum Age: 15 Confirmations (PoS-v3) | 30 Minutes (PoS-v2)
	Port: 19427
	RPC Port: 18695


BUILD LINUX
-----------
### Compiling CampusCash "SatoshiCore" daemon on Ubunutu 18.04 LTS Bionic
### Note: guide should be compatible with other Ubuntu versions from 14.04+

### Become poweruser
```
sudo -i
```
### CREATE SWAP FILE FOR DAEMON BUILD (if system has less than 2GB of RAM)
```
cd ~; sudo fallocate -l 3G /swapfile; ls -lh /swapfile; sudo chmod 600 /swapfile; ls -lh /swapfile; sudo mkswap /swapfile; sudo swapon /swapfile; sudo swapon --show; sudo cp /etc/fstab /etc/fstab.bak; echo '/swapfile none swap sw 0 0' | sudo tee -a /etc/fstab
```

### Dependencies install
```
cd ~; sudo apt-get install -y ntp git build-essential libssl-dev libdb-dev libdb++-dev libboost-all-dev libqrencode-dev libcurl4-openssl-dev curl libzip-dev; apt-get update -y; apt-get install -y git make automake build-essential libboost-all-dev; apt-get install -y yasm binutils libcurl4-openssl-dev openssl libssl-dev; sudo apt-get install -y libgmp-dev; sudo apt-get install -y libtool; sudo apt-get install qt5-default; sudo apt-get install qttools5-dev-tools;
```

### Dependencies build and link
```
cd ~; wget http://download.oracle.com/berkeley-db/db-6.2.32.NC.tar.gz; tar zxf db-6.2.32.NC.tar.gz; cd db-6.2.32.NC/build_unix; ../dist/configure --enable-cxx --disable-shared; make; sudo make install; sudo ln -s /usr/local/BerkeleyDB.6.2/lib/libdb-6.2.so /usr/lib/libdb-6.2.so; sudo ln -s /usr/local/BerkeleyDB.6.2/lib/libdb_cxx-6.2.so /usr/lib/libdb_cxx-6.2.so; export BDB_INCLUDE_PATH="/usr/local/BerkeleyDB.6.2/include"; export BDB_LIB_PATH="/usr/local/BerkeleyDB.6.2/lib"
```

### GitHub pull (Source Download)
```
cd ~; git clone https://github.com/SaltineChips/CampusCash CampusCash
```

### Build CampusCash daemon
```
cd ~; cd ~/CampusCash/src; chmod a+x obj; chmod a+x leveldb/build_detect_platform; chmod a+x secp256k1; chmod a+x leveldb; chmod a+x ~/CampusCash/src; chmod a+x ~/CampusCash; make -f makefile.unix USE_UPNP=-; cd ~; cp -r ~/CampusCash/src/CampusCashd /usr/local/bin/CampusCashd;
```

### (Optional) Build CampusCash-QT (GUI wallet) on Linux 

**All previous steps must be completed first.**

If you recompiling some other time you don't have to repeat previous steps, but need to define those variables. Skip this command if this is your first build and previous steps were performed in current terminal session.
```
export BDB_INCLUDE_PATH="/usr/local/BerkeleyDB.6.2/include"; export BDB_LIB_PATH="/usr/local/BerkeleyDB.6.2/lib"
```

With UPNP:

```
cd ~; cd ~/CampusCash; qmake -qt=qt5; make
```

**Recommended Without** UPNP:

```
cd ~; cd ~/CampusCash; qmake -qt=qt5 USE_UPNP=-; make
```



### Create config file for daemon
```
cd ~; sudo ufw allow 19427/tcp; sudo ufw allow 18695/tcp; sudo ufw allow 22/tcp; sudo mkdir ~/.CCASH; cat << "CONFIG" >> ~/.CCASH/CampusCash.conf
listen=1
server=1
daemon=1
testnet=0
rpcuser=CCASHrpcuser
rpcpassword=SomeCrazyVeryVerySecurePasswordHere
rpcport=18695
port=19427
rpcconnect=127.0.0.1
rpcallowip=127.0.0.1
addnode=45.77.210.8
addnode=45.77.210.8:19427
addnode=45.77.210.234
addnode=45.77.210.234:19427
addnode=192.168.1.14
addnode=192.168.1.14:19427
addnode=104.238.156.128
addnode=104.238.156.128:19427
addnode=66.42.71.176
addnode=66.42.71.176:19427
addnode=110.109.107.71
addnode=110.109.107.71:19427
addnode=152.89.106.95
addnode=152.89.106.95:19427
addnode=188.68.33.155
addnode=188.68.33.155:19427
addnode=5.45.108.38
addnode=5.45.108.38:19427
addnode=138.197.161.183
addnode=138.197.161.183:19427
addnode=157.230.107.144
addnode=157.230.107.144:19427
addnode=137.220.34.237
addnode=137.220.34.237:19427
addnode=184.166.67.221
addnode=184.166.67.221:19427
addnode=167.99.88.37
addnode=167.99.88.37:19427
CONFIG
chmod 700 ~/.CCASH/CampusCash.conf; chmod 700 ~/.CCASH; ls -la ~/.CCASH
```

### Run CampusCash daemon
```
cd ~; CampusCashd; CampusCashd getinfo
```

### Troubleshooting
### for basic troubleshooting run the following commands when compiling:
### this is for minupnpc errors compiling

```
make clean -f makefile.unix USE_UPNP=-
make -f makefile.unix USE_UPNP=-
```
### Updating daemon in bin directory
```
cd ~; cp -r ~/CampusCash/src/CampusCashd /usr/local/bin
```

License
-------

CampusCash [CCASH] is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/SaltineChips/CampusCash/Tags) are created
regularly to indicate new official, stable release versions of CampusCash [CCASH].

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).

The developer [mailing list](https://lists.linuxfoundation.org/mailman/listinfo/bitcoin-dev)
should be used to discuss complicated or controversial changes before working
on a patch set.

Developer Discord can be found at https://discord.gg/Jp2ujYH.

Testing
-------

Testing and code review is the bottleneck for development; we get more pull
requests than we can review and test on short notice. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people
lots of money.

### Automated Testing

Developers are strongly encouraged to write [unit tests](/doc/unit-tests.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`

There are also [regression and integration tests](/qa) of the RPC interface, written
in Python, that are run automatically on the build server.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.
