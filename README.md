### profile   
Highport is a high-concurrency software architecture for encapsulating neural network inference. It has achieved the first place in the ESWEEK 2023 skin disease detection competition. Here is an introduction to its software architecture and some trick optimizations:

• Decryption Module: The trained model files are encrypted. To prevent them from being stolen by others, the encryption method uses XOR calculation between the password and the binary content of the weight files. The decrypted files are subjected to MD5 verification, and if it matches the predicted MD5 checksum, the decryption is considered correct.

• File Index Creation Module: The image paths and filenames for inference are stored in a table and indexed with numbers, which greatly shortens the index time for reading files.

• Concurrent Processing Module: If there is only one CPU core and there is no blocking during the task execution, the efficiency of concurrent processing tasks by threads will be lower than that of single-thread processing tasks because multithreading requires overhead for task stacks, scheduling, and locks. Since the required hardware platform is a 4-core CPU, only 4 threads are created for concurrency to achieve the highest efficiency.

• Image Processing Module: The inference efficiency of loading four models is higher than that of loading one model with four CPU concurrent lock protection.

### build 
'mkdir build; cd build; cmake ..; make'

### requirement  
git clone https://github.com/alibaba/MNN.git 
mkidr build; cd build; cmake ..; make ; sudo make install  

### port to pi_borad
modify CMakeFileList.txt lib position

### performance
43.95ms handle 1 picture(pixel 96 * 96) in raspberry4B platform

### license
MIT by Jim
