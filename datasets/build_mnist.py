import os
from DennDBLib.creator import mnist
from DennDBLib.utils import exists_or_download

MNIST_PATH = os.path.join("_dbcache_","mnist")
     
def get_mnist(mnist_path):
    if not os.path.exists(mnist_path):
        os.makedirs(mnist_path)
    exists_or_download(os.path.join(mnist_path,"train-images-idx3-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"train-labels-idx1-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"t10k-images-idx3-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"t10k-labels-idx1-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz")
    
def main():
    get_mnist(MNIST_PATH)
    print("Start generation: mnist.db.gz")
    mnist('mnist.db', 
        source_folder = MNIST_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        depth=1, 
        normalized = False
    )
    print("Start generation: mnist_novalidation.db.gz")
    mnist('mnist_novalidation.db', 
        source_folder = MNIST_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        depth=1, 
        normalized = False, 
        validation_size=0
    )
    print("Start generation: mnist_normalized.db.gz")
    mnist('mnist_normalized.db', 
        source_folder = MNIST_PATH,
        batch_size=2000, 
        dest_folder=os.getcwd(), 
        depth=1, 
        normalized = True
    )
    print("Start generation: mnist_novalidation_normalized.db.gz")
    mnist('mnist_novalidation_normalized.db', 
           source_folder = MNIST_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           depth=1, 
           normalized = True, 
           validation_size=0
    )
    print("Start generation: mnist_cpvalidation.db.gz")
    mnist('mnist_cpvalidation.db', 
           source_folder = MNIST_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           depth=1, 
           normalized = False,
           validation_as_copy = True
    )
    print("Start generation: mnist_cpvalidation_normalized.db.gz")
    mnist('mnist_cpvalidation_normalized.db', 
           source_folder = MNIST_PATH,
           batch_size=2000, 
           dest_folder=os.getcwd(), 
           depth=1, 
           normalized = True,
           validation_as_copy = True
    )


if __name__ == '__main__':
    main()