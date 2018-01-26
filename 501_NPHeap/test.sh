cd benchmark
make
sudo make install
cd ..
cd library
make
sudo make install
cd ..
cd kernel_module
make
sudo make install
cd ..
sudo insmod kernel_module/npheap.ko
sudo chmod 777 /dev/npheap
./benchmark/benchmark 256 8192 8





