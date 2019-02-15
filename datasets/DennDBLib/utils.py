import os
import gzip
from io import StringIO
from urllib import request

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
           