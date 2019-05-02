import os
from DennDBLib.creator import nbit_parity 

def main():
    for nbit in range(5,19):        
        print("Start generation: xor{}.db.gz".format(nbit))
        nbit_parity("xor{}.db".format(nbit), nbit=nbit, 
        validation_all_values=False, 
        test_all_values=False, 
        batch_size=1000)  
        print("Start generation: xor{}_cpval.db.gz".format(nbit))
        nbit_parity("xor{}_cpval.db".format(nbit), nbit=nbit, 
        validation_all_values=False,
        validation_as_copy=True, 
        validation_size=0.2,
        test_all_values=False, 
        batch_size=1000)

if __name__ == '__main__':
    main()