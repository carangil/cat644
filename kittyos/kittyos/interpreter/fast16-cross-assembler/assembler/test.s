
ldi a $13
ldi b $5600
add b
swp b
ldi a $7899
syscall 4

jmpr @end

string 'ABC

syscall 1
end:
syscall 2

halt:
jmpr @halt


