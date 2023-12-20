### high concurrency to handle a mount of picture data
In FAIR ML DESIGN CONTEST at ESWEEK 2023 is the final top 1 of the competition!

### build 
'mkdir build; cd build; cmake ..; make'

### requment
git clone https://github.com/alibaba/MNN.git 
mkidr build; cd build; cmake ..; make ; sudo make install  

### port to pi_borad
modify CMakeFileList.txt lib position

### performance
80ms handle 1 picture(pixel 128 * 128) in raspberry4B platform

### license
MIT by Jim
