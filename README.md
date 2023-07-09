------------------------------
INSTRUCTION
------------------------------

1) Download and the unzip the tar file. Go the base directory of the project, then type "cmake . -DCMAKE_BUILD_TYPE=Release"
2) Type "make"
3) 2 binaries are generated in the bin/Release folder:
    - Kraken: The orderbook server. You can run the binary from the base directory by running "./bin/Release/Kraken". It is listening on port 35000
    - client: A simple program that takes a filename as command line parameter and send the content of the file line by line through UDP to port 35000. You can run it by typing "./bin/Release/client input.txt"

------------------------------
Issues/Assumption
------------------------------

1) From the document, the cancel ack message and the new order ack message have the same format (A, userId, userOrderId). However, the cancel ack message in the output file for scenario 15 has a different format where it starts with the character "C" and has an extra value at the end. I am not sure which one is correct, but I stick with the format defined in the document, which means the cancel ack and new order ack has the same format.
2) For the output of scenario 1, the second trade message (T, 2, 103, 1, 1, 10, 100) seems to have the buy/sell userid and userOrderId flipped
