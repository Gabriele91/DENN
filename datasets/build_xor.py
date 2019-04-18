import os
from DennDBLib.creator import nbit_parity 

def main():
    for nbit in range(5,13):        
        print("Start generation: xor{}.db.gz".format(nbit))
        nbit_parity("xor{}.db".format(nbit), nbit=nbit, validation_all_values=True, test_all_values=True, batch_size=1000)

if __name__ == '__main__':
    main()