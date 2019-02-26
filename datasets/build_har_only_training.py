import os
from DennDBLib.creator import har_only_training
from DennDBLib.utils import exists_or_download

HAR_PATH = os.path.join("_dbcache_","har")
     
def get_har(har_path):
    if not os.path.exists(har_path):
        os.makedirs(har_path)
    exists_or_download(os.path.join(har_path,"UCI HAR Dataset.zip"),"https://archive.ics.uci.edu/ml/machine-learning-databases/00240/UCI%20HAR%20Dataset.zip")
    
def main():
    get_har(HAR_PATH)
    print("Start generation: har_ot.db.gz")
    har_only_training('har_ot.db', 
        source_folder = HAR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: har_ot_novalidation.db.gz")
    har_only_training('har_ot_novalidation.db', 
        source_folder = HAR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False, 
        validation_size=0,
        test_size=0.15
    )
    print("Start generation: har_ot_normalized.db.gz")
    har_only_training('har_ot_normalized.db', 
        source_folder = HAR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: har_ot_novalidation_normalized.db.gz")
    har_only_training('har_ot_novalidation_normalized.db', 
        source_folder = HAR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True, 
        validation_size=0,
        test_size=0.15
    )
    print("Start generation: har_ot_cpvalidation.db.gz")
    har_only_training('har_ot_cpvalidation.db', 
        source_folder = HAR_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False,
        validation_as_copy = True,
        validation_size=0.15,
        test_size=0.15
    )
    print("Start generation: har_ot_cpvalidation_normalized.db.gz")
    har_only_training('har_ot_cpvalidation_normalized.db', 
           source_folder = HAR_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           normalized = True,
           validation_as_copy = True,
           validation_size=0.15,
           test_size=0.15
    )



if __name__ == '__main__':
    main()