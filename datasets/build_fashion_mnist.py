import os
from DennDBLib.creator import fashion_mnist
from DennDBLib.utils import exists_or_download

MNIST_PATH = os.path.join("_dbcache_","fashion_mnist")
     
def get_mnist(mnist_path):
    if not os.path.exists(mnist_path):
        os.makedirs(mnist_path)
    exists_or_download(os.path.join(mnist_path,"train-images-idx3-ubyte.gz"),"http://fashion-mnist.s3-website.eu-central-1.amazonaws.com/train-images-idx3-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"train-labels-idx1-ubyte.gz"),"http://fashion-mnist.s3-website.eu-central-1.amazonaws.com/train-labels-idx1-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"t10k-images-idx3-ubyte.gz"), "http://fashion-mnist.s3-website.eu-central-1.amazonaws.com/t10k-images-idx3-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"t10k-labels-idx1-ubyte.gz"), "http://fashion-mnist.s3-website.eu-central-1.amazonaws.com/t10k-labels-idx1-ubyte.gz")
    
def main():
    get_mnist(MNIST_PATH)
    print("Start generation: fashion_mnist.db.gz")
    fashion_mnist('fashion_mnist.db', 
        source_folder = MNIST_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False
    )
    print("Start generation: fashion_mnist_novalidation.db.gz")
    fashion_mnist('fashion_mnist_novalidation.db', 
        source_folder = MNIST_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = False, 
        validation_size=0
    )
    print("Start generation: fashion_mnist_normalized.db.gz")
    fashion_mnist('fashion_mnist_normalized.db', 
        source_folder = MNIST_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        normalized = True
    )
    print("Start generation: fashion_mnist_novalidation_normalized.db.gz")
    fashion_mnist('fashion_mnist_novalidation_normalized.db', 
            source_folder = MNIST_PATH,
            batch_size=2000, 
            dest_folder=os.getcwd(), 
            normalized = True, 
            validation_size=0
    )
    print("Start generation: fashion_mnist_cpvalidation.db.gz")
    fashion_mnist('fashion_mnist_cpvalidation.db', 
           source_folder = MNIST_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           normalized = False,
           validation_as_copy = True
    )
    print("Start generation: fashion_mnist_cpvalidation_normalized.db.gz")
    fashion_mnist('fashion_mnist_cpvalidation_normalized.db', 
           source_folder = MNIST_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           normalized = True,
           validation_as_copy = True
    )


if __name__ == '__main__':
    main()
