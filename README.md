------------------------------
INSTRUCTION
------------------------------

1) Download and the unzip the tar file. Go the base directory of the project, then type "cmake . -DCMAKE_BUILD_TYPE=Release"
2) Type "make"
3) The binary is generated in the bin folder. You can run the binary from the base directory by running "./bin/Kraken"

------------------------------
Issues/Assumption
------------------------------

From the document, the cancel ack message and the new order ack message have the same format (A, userId, userOrderId). However, the cancel ack message in the output file for scenario 15 has a different format where it starts with the character "C" and has an extra value at the end. I am not sure which one is correct, but I stick with the format defined in the document, which means the cancel ack and new order ack has the same format.
