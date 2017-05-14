# Go-Back-N-ARQ
Go-Back-N ARQ using raw sockets. The mac addresses of the sender and receiver is used for sending the frames.  

There are two threads on the server side - one sends the frames and implements go-back-n ARQ while the other one simply receives the acknowledgements. The reciever receives the frames and sends the necessary acknowledgements.

 A function (send_frame) to send a frame which takes the payload, serialNo and acknowledgementNo as the parameters is created and is present in send_raw.h
