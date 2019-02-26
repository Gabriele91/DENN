import os
from DennDBLib.creator import esr_1_2
from DennDBLib.utils import exists_or_download

ESR_PATH = os.path.join("_dbcache_","esr")
     
def get_esr(esr_path):
    if not os.path.exists(esr_path):
        os.makedirs(esr_path)
    exists_or_download(os.path.join(esr_path,"data.csv"),"https://archive.ics.uci.edu/ml/machine-learning-databases/00388/data.csv")
    
def main():
    get_esr(ESR_PATH)
    print("Start generation: esr_1_2.db.gz")
    esr_1_2('esr_1_2.db', 
        source_folder = ESR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: esr_1_2_novalidation.db.gz")
    esr_1_2('esr_1_2_novalidation.db', 
        source_folder = ESR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False, 
        validation_size=0,
        test_size=0.15
    )
    print("Start generation: esr_1_2_normalized.db.gz")
    esr_1_2('esr_1_2_normalized.db', 
        source_folder = ESR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: esr_1_2_novalidation_normalized.db.gz")
    esr_1_2('esr_1_2_novalidation_normalized.db', 
        source_folder = ESR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True, 
        validation_size=0,
        test_size=0.15
    )
    print("Start generation: esr_1_2_cpvalidation.db.gz")
    esr_1_2('esr_1_2_cpvalidation.db', 
        source_folder = ESR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False,
        validation_as_copy = True,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: esr_1_2_cpvalidation_normalized.db.gz")
    esr_1_2('esr_1_2_cpvalidation_normalized.db', 
           source_folder = ESR_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           normalized = True,
           validation_as_copy = True,
           validation_size=0.15,
           test_size=0.15
    )



if __name__ == '__main__':
    main()