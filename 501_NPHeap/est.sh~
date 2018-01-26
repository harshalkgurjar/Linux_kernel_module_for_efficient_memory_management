cat *.log > trace
#echo "done with cat"
sort -n -k 3 trace > sorted_trace
#echo "done with sort"
./benchmark/validate 256 8192 < sorted_trace
rm -f *.log
sudo rmmod npheap
