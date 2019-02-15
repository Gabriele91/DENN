import os
import gzip
from io import StringIO
from urllib import request
from DennDBLib.creator import mnist

MNIST_PATH = "_dbcache_/mnist"

def exists_or_download(filename, url, decompressedFile = False):
    if not os.path.exists(filename):
        print("Download:", filename, "from:",url)
        response = request.urlopen(url)
        if decompressedFile:
            compressedFile = StringIO()
            compressedFile.write(response.read())
            compressedFile.seek(0)
            decompressedFile = gzip.GzipFile(fileobj=compressedFile, mode='rb')
            with open(filename,'wb') as output:
                output.write(decompressedFile.read())
        else:
            with open(filename,'wb') as output:
                output.write(response.read())
                
def get_mnist(mnist_path):
    if not os.path.exists(mnist_path):
        os.makedirs(mnist_path)
    exists_or_download(os.path.join(mnist_path,"train-images-idx3-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/train-images-idx3-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"train-labels-idx1-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/train-labels-idx1-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"t10k-images-idx3-ubyte.gz"),"http://yann.lecun.com/exdb/mnist/t10k-images-idx3-ubyte.gz")
    exists_or_download(os.path.join(mnist_path,"t10k-labels-idx1-ubyte.gZ"),"http://yann.lecun.com/exdb/mnist/t10k-labels-idx1-ubyte.gz")
    
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


if __name__ == '__main__':
    main()
