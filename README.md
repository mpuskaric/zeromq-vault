# zeromq-vault
simple client-server with serialization. Check subfolders for other projects

Prerequisites: msgpack, libzmq, cppzmq 

# Compilation
g++ -o client hwclient.cpp -lzmq

g++ -o server hwserver.cpp -lzmq
