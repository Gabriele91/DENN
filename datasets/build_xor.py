import os
from DennDBLib.creator import nbit_parity 

def main():
    for binary in [False, True]:
        for nbit in range(5,19):        
            print("Start generation: xor{}{}.db.gz".format(nbit, "_bin" if  binary else ""))
            nbit_parity("xor{}{}.db".format(nbit, "_bin" if  binary else ""), nbit=nbit, 
                validation_all_values=False, 
                test_all_values=False, 
                batch_size=1000,
                binary = binary
            )  
            print("Start generation: xor{}_cpval{}.db.gz".format(nbit, "_bin" if  binary else ""))
            nbit_parity("xor{}_cpval{}.db".format(nbit, "_bin" if  binary else ""), nbit=nbit, 
                validation_all_values=False,
                validation_as_copy=True, 
                validation_size=0.2,
                test_all_values=False, 
                batch_size=1000,
                binary = binary
            )

if __name__ == '__main__':
    main()