
make
#mpiexec -n 2 ./guessword 1-passwd.txt 1-shadow.txt

mpiexec -n 2 ./guessword Files/Userdata/sample1-passwd.txt Files/Userdata/sample1-shadow.txt
