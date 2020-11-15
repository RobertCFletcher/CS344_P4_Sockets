# CS344_P4_Sockets
Sends and receives encoded messages from a server using network sockets

keygen: 
  -Generates a random string to use for the key 
  -Usage Example: keygen 70000 > key70000 which will create a keygen file

otp_enc:
  -This is the client portion for encoding ciphertext, runs once when called to talk to server and gets an encoded message back from the server

otp_enc_d:
  -is the server portion for encoding ciphertext, runs constantly until stopped

otp_dec:
  -is the client portion for decoding ciphertext, runs once when called to talk to server and gets a decoded message back from the server
  
otp_dec_d
  -This is the server portion for decoding ciphertext, runs constantly until stopped
