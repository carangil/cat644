
ldi a $13
ldi b $5600
add b
swp b
ldi a 1
syscall

done:
jmpr @done
