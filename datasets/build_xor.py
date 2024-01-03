import os
from DennDBLib.creator import nbit_parity 
import argparse

def main(start,end):
    for binary in [False, True]:
        for nbit in range(start,end):        
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
    parser = argparse.ArgumentParser(description='Generate xor datasets between [start,end)')
    parser.add_argument('--start', type=int, default=5)
    parser.add_argument('--end', type=int, default=19)
    args = parser.parse_args()
    main(args.start,args.end)