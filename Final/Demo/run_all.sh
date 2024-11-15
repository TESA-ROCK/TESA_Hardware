#!/bin/bash

# Compile all C files with the required libraries
# gcc -Wall -Wextra -g -o 1_1_Sound_detection 1_1_Sound_detection.c -lm -lsqlite3 -lasound -lpthread
# gcc -Wall -Wextra -g -o 1_2_segment_datasize 1_2_segment_datasize.c -lm -lsqlite3 -lpthread -lcjson
# gcc -Wall -Wextra -g -o 6_2_SendCodeToPi 6_2_SendCodeToPi.c -lm -lsqlite3 -lpthread -lcjson
# gcc -Wall -Wextra -g -o 7_3_Classify 7_3_Classify.c -lm -lsqlite3 -lpthread -lcjson -lssl -lcrypto

gcc -o 1_1_Sound_detection 1_1_Sound_detection.c -lm -lsqlite3 -lasound -lpthread
gcc -o 1_2_segment_datasize 1_2_segment_datasize.c -lpaho-mqtt3c -lsqlite3 -lcjson -lasound -lm
gcc -o 6_2_SendCodeToPi 6_2_SendCodeToPi.c -lpaho-mqtt3c -lsqlite3 -lcjson -lm
gcc -o 7_3_Classify 7_3_Classify.c -lpaho-mqtt3c -lsqlite3 -lcjson -lm
#gcc 2_2_Encode_Data.c -o 2_2_Encode_Data -lpaho-mqtt3c -lssl -lcrypto -lsqlite3 -lcjson -lpthread
gcc -o 2_2_Encode_Data 2_2_Encode_Data.c -lpaho-mqtt3c -lssl -lcrypto -lsqlite3 -lcjson -lpthread

# Run all the executables sequentially
#./1_1_Sound_detection&
./1_2_segment_datasize&
./2_2_Encode_Data&
./6_2_SendCodeToPi&
./7_3_Classify&


# Wait for all background processes to finish
wait

# chmod +x run_all.sh
# ./run_all.sh