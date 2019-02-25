import os
from DennDBLib.creator import wdbc
from DennDBLib.utils import exists_or_download

WDBC_PATH = os.path.join("_dbcache_","wdbc")
     
def get_wdbc(wdbc_path):
    if not os.path.exists(wdbc_path):
        os.makedirs(wdbc_path)
    exists_or_download(os.path.join(wdbc_path,"wdbc.data"),"https://archive.ics.uci.edu/ml/machine-learning-databases/breast-cancer-wisconsin/wdbc.data")
    
def main():
    get_wdbc(WDBC_PATH)
    print("Start generation: wdbc.db.gz")
    wdbc('wdbc.db', 
        source_folder = WDBC_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: wdbc_novalidation.db.gz")
    wdbc('wdbc_novalidation.db', 
        source_folder = WDBC_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False, 
        validation_size=0,
        test_size=0.15
    )
    print("Start generation: wdbc_normalized.db.gz")
    wdbc('wdbc_normalized.db', 
        source_folder = WDBC_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: wdbc_novalidation_normalized.db.gz")
    wdbc('wdbc_novalidation_normalized.db', 
        source_folder = WDBC_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True, 
        validation_size=0,
        test_size=0.15
    )
    print("Start generation: wdbc_cpvalidation.db.gz")
    wdbc('wdbc_cpvalidation.db', 
        source_folder = WDBC_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False,
        validation_as_copy = True,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: wdbc_cpvalidation_normalized.db.gz")
    wdbc('wdbc_cpvalidation_normalized.db', 
           source_folder = WDBC_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           normalized = True,
           validation_as_copy = True,
           validation_size=0.15,
           test_size=0.15
    )



if __name__ == '__main__':
    main()