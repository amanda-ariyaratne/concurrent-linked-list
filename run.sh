iterations=10
thread_count=8
p_mem=0.99
p_ins=0.005
p_del=0.005

echo $(gcc -g -Wall -o serial serial.c -lm && ./serial ${p_mem} ${p_ins} ${p_del} ${iterations} ${thread_count})
echo $(gcc -g -Wall -o mutex mutex.c -lpthread -lm && ./mutex ${p_mem} ${p_ins} ${p_del} ${iterations} ${thread_count}) 
echo $(gcc -g -Wall -o read_write_lock read_write_lock.c -lpthread -lm && ./read_write_lock ${p_mem} ${p_ins} ${p_del} ${iterations} ${thread_count}) 