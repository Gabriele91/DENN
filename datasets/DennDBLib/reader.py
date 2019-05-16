"""Reader module contains only dataset loaders
that are used to iterate over items."""
import csv
import gzip
import io
import struct
import zipfile
from collections import OrderedDict
from os import path

import numpy as np

from .var import NP2STRUCT

__all__ = ["Dataset", "Resource", "Container", "READER_LIST"]

def check_min_max(rows):
    """Check min and max in a csv table."""
    min_max = {}
    for row in rows:
        for key, value in row.items():
            try:
                tmp = float(value)
                if key not in min_max:
                    min_max[key] = [0, 0]
                if tmp > min_max[key][1]:
                    min_max[key][1] = tmp
                elif tmp < min_max[key][0]:
                    min_max[key][0] = tmp
            except ValueError:
                pass
    return min_max




class Resource(object):

    """Dataset resource."""

    def __init__(self, attributes, outputs):
        assert isinstance(
            attributes, np.ndarray), "attributes have to be a numpy array"
        assert isinstance(
            outputs, np.ndarray), "outputs have to be a numpy array"
        self.attributes = attributes
        self.outputs = outputs
        self.__idx = 0

    def __len__(self):
        return len(self.attributes)

    def __iter__(self):
        self.__idx = 0
        while self.__idx < len(self.attributes):
            yield (self.attributes[self.__idx], self.outputs[self.__idx])
            self.__idx += 1

    def __getitem__(self, idx):
        return (self.attributes[idx].copy(), self.outputs[idx].copy())

    def __setitem__(self, idx, val):
        self.attributes[idx], self.outputs[idx] = val
        return (self.attributes[idx].copy(), self.outputs[idx].copy())

    def repeat(self, num_rep, target='attributes'):
        """Add a third dimension and duplicate data.

        Example:
            - with 3 repetitions:

                from [ [1, 2], [3, 4] ]

                to [ 
                    [ [1, 2], [3, 4] ],
                    [ [1, 2], [3, 4] ],
                    [ [1, 2], [3, 4] ]
                ]

        Note:
            np.array([tmp] * num_rep) is more performing than
            np.tile(tmp, (num_rep, 1, 1))

            tmp = np.array([ [1, 2], [3, 4] ])

            %time _ = [np.tile(tmp, (3, 1, 1)) for _ in range(1000000)]
            CPU times: user 6.9 s, sys: 189 ms, total: 7.09 s
            Wall time: 7.14 s

            %time _ = [np.array([tmp]*3) for _ in range(1000000)]
            CPU times: user 3.03 s, sys: 147 ms, total: 3.18 s
            Wall time: 3.19 s
        """
        if target == 'attributes':
            self.attributes = np.array([self.attributes] * num_rep)
        elif target == 'outputs':
            self.outputs = np.array([self.outputs] * num_rep)
        else:
            raise Exception("Unrecognized target '{}'".format(target))
        return self

    def shuffle(self, seed=None):
        """Shuffle resource items."""
        np.random.seed(seed)
        indexes = np.random.permutation(len(self.attributes))
        self.attributes = self.attributes[indexes]
        self.outputs = self.outputs[indexes]
        return self

    def extract(self, indexes):
        """Remove items from this resource."""
        attributes = self.attributes[indexes].copy()
        outputs = self.outputs[indexes].copy()
        self.attributes = np.delete(self.attributes, indexes, 0)
        self.outputs = np.delete(self.outputs, indexes, 0)
        return Resource(attributes, outputs)

    def copy_fields(self, indexes):
        """Copy items from this resource."""
        attributes = self.attributes[indexes].copy()
        outputs = self.outputs[indexes].copy()
        return Resource(attributes, outputs)

    def split(self, num_of_splits):
        """Split the current resource."""
        attributes = np.array_split(self.attributes, num_of_splits)
        outputs = np.array_split(self.outputs, num_of_splits)
        return [Resource(attributes[idx], outputs[idx]) for idx in range(len(attributes))]

    def copy(self):
        """Copy all the items of this resource."""
        return Resource(self.attributes.copy(), self.outputs.copy())

    def convert_type(self, type_):
        """Convert data type."""
        assert type_ == 'double' or type_ == 'float', "Output data can be double or float"
        if type_ == 'double':
            self.attributes = self.attributes.astype(np.float64)
            self.outputs = self.outputs.astype(np.float64)
        elif type_ == 'float':
            self.attributes = self.attributes.astype(np.float32)
            self.outputs = self.outputs.astype(np.float32)

    def to_bin(self):
        """Convert to binary this resource."""
        values = [self.attributes.flatten(), self.outputs.flatten()]
        data = np.concatenate(values, axis=0)
        return struct.pack("{}{}".format(
            data.size,
            NP2STRUCT[self.attributes.dtype]
        ), *data)

    def __add__(self, other):
        if len(self.attributes) > 0:
            attributes = np.concatenate(
                [self.attributes, other.attributes], axis=0)
            outputs = np.concatenate([self.outputs, other.outputs], axis=0)
        else:
            attributes = other.attributes
            outputs = other.outputs
        return Resource(attributes, outputs)

    def __iadd__(self, other):
        if len(self.attributes) > 0:
            self.attributes = np.append(
                self.attributes, other.attributes, axis=0)
            self.outputs = np.append(self.outputs, other.outputs, axis=0)
        else:
            self.attributes = other.attributes
            self.outputs = other.outputs
        return self

class Container(object):

    """Special container for Train, Validation and Test sets."""

    def __init__(self, *args, **kwargs):
        self.resources = []

    def insert(self, source):
        """Insert a resource into this container."""
        if isinstance(source, list):
            self.resources.extend(source)
        else:
            self.resources.append(source)
        return self

    def delete(self, idx):
        """Remove a resource."""
        self.resources.pop(idx)
        return self

    def __iter__(self):
        for resource in self.resources:
            yield resource

    def __len__(self):
        return sum([len(resource) for resource in self.resources])

    def __getitem__(self, idx):
        target_idx = idx
        for resource in self.resources:
            if target_idx < len(resource):
                return resource[target_idx]
            target_idx -= len(resource)
        raise IndexError()

    def __setitem__(self, idx, val):
        target_idx = idx
        for resource in self.resources:
            if target_idx < len(resource):
                resource[target_idx] = val
                return resource[target_idx]
            target_idx -= len(resource)
        raise IndexError()

class Dataset(object):

    """Base dataset object."""

    def __init__(self, normalized=False, onehot=False, kind="reg"):
        assert kind == "cls" or kind == "reg", "kind can be 'cls' (classification) or 'reg' (regression)"
        self.kind = kind
        self.normalized = normalized
        self.onehot = onehot
        self._train = Container()
        self._validation = Container()
        self._test = Container()
        self.data = OrderedDict([
            ('train', self._train),
            ('validation', self._validation),
            ('test', self._test)
        ])

    @property
    def train(self):
        """Train set."""
        return self._train

    @property
    def validation(self):
        """Validation set."""
        return self._validation

    @property
    def test(self):
        """Test set."""
        return self._test

    def insert(self, source, target="train"):
        """Insert a resoure in a proper container."""
        assert isinstance(
            source, Resource), "source have to be a Resource object"
        assert target == "train" or target == "validation" or target == "test", "target can be 'train', 'validation' or 'test'"
        cur_target = self.data[target]
        cur_target.insert(source)

    def __iter__(self):
        for set_ in self.data.values():
            for resource in set_.resources:
                for data in resource:
                    yield data

    def __getitem__(self, idx):
        target_idx = idx
        for set_ in self.data.values():
            for resource in set_.resources:
                if target_idx < len(resource):
                    return resource[target_idx]
                target_idx -= len(resource)
        raise IndexError()

    def __len__(self):
        return sum(
            [len(set_) for set_ in self.data.values()]
        )

class MAGICGammaTelescopeDataset(Dataset):

    """MAGIC Gamma Telescope dataset."""

    def __init__(self, magic_source_folder=None, normalized=True, onehot=True):
        super(MAGICGammaTelescopeDataset, self).__init__(
            normalized=normalized, onehot=onehot, kind='cls')

        data = []
        labels = []

        attributes = [
            'fLength',
            'fWidth',
            'fSize',
            'fConc',
            'fConc1',
            'fAsym',
            'fM3Long',
            'fM3Trans',
            'fAlpha',
            'fDist',
            'class'
        ]

        with open(path.join(magic_source_folder, "magic04.data"), "rb") as csvfile:
            file_wrapper = io.TextIOWrapper(csvfile, newline='')
            reader = csv.DictReader(file_wrapper,
                                    delimiter=',',
                                    fieldnames=attributes
                                    )
            rows = list(reader)

            min_max = check_min_max(rows)

            for row in rows:
                new_data = []
                for attr in attributes[:-1]:
                    tmp = float(row[attr])
                    if normalized:
                        tmp = (
                            tmp - min_max[attr][0]) / (
                            min_max[attr][1] - min_max[attr][0]
                        )
                    new_data.append(tmp)
                data.append(np.array(new_data))

                if onehot:
                    if row['class'] == 'g':
                        labels.append(np.array([0, 1]))
                    else:
                        labels.append(np.array([1, 0]))
                else:
                    labels.append(1 if row['class'] == 'g' else 0)

        data = np.array(data)
        labels = np.array(labels)

        self.insert(Resource(data, labels))

class QSARDataset(Dataset):

    """QSAR dataset."""

    def __init__(self, qsar_source_folder=None, normalized=True, onehot=True):
        super(QSARDataset, self).__init__(
            normalized=normalized, onehot=onehot, kind='cls')

        data = []
        labels = []

        attributes = [
            'SpMax_L',
            'J_Dz',
            'nHM',
            'F01',
            'F04',
            'NssssC',
            'nCb',
            'C',
            'nCp',
            'nO',
            'F03',
            'SdssC',
            'from',
            'LOC',
            'SM6_L',
            'F03',
            'Me',
            'Mi',
            'nN',
            'nArNO2',
            'nCRX3',
            'SpPosA_B',
            'nCIR',
            'B01',
            'B03',
            'N',
            'SpMax_A',
            'Psi_i_1d',
            'B04',
            'SdO',
            'TI2_L',
            'nCrt',
            'C',
            'F02',
            'nHDon',
            'SpMax_B',
            'Psi_i_A',
            'nN',
            'SM6_B',
            'nArCOOR',
            'nX',
            'class'
        ]

        with open(path.join(qsar_source_folder, "biodeg.csv"), "rb") as csvfile:
            file_wrapper = io.TextIOWrapper(csvfile, newline='')
            reader = csv.DictReader(file_wrapper,
                                    delimiter=';',
                                    fieldnames=attributes
                                    )
            rows = list(reader)

            min_max = check_min_max(rows)

            for row in rows:
                new_data = []
                for attr in attributes[:-1]:
                    tmp = float(row[attr])
                    if normalized:
                        tmp = (
                            tmp - min_max[attr][0]) / (
                            min_max[attr][1] - min_max[attr][0]
                        )
                    new_data.append(tmp)
                data.append(np.array(new_data))
                if onehot:
                    if row['class'] == 'RB':
                        labels.append(np.array([0, 1]))
                    else:
                        labels.append(np.array([1, 0]))
                else:
                    labels.append(1 if row['class'] == 'RB' else 0)

        data = np.array(data)
        labels = np.array(labels)

        self.insert(Resource(data, labels))

class WDBCDataset(Dataset):
    
    def __init__(self, wdbc_source_folder=None, normalized=True, onehot=True):
        super(WDBCDataset, self).__init__(normalized=normalized, onehot=onehot, kind='cls')
        data = []
        labels = []
        attributes = ['user','class']
        attributes.extend([str(x+1) for x in range(30)])

        with open(path.join(wdbc_source_folder, "wdbc.data"), "rb") as csvfile:
            file_wrapper = io.TextIOWrapper(csvfile, newline='')
            reader = csv.DictReader(file_wrapper, delimiter=',', fieldnames=attributes)
            rows = list(reader)
            min_max = check_min_max(rows)
            for row in rows:
                new_data = []
                for attr in attributes[2:]:
                    tmp = float(row[attr])
                    if normalized:
                        tmp = (tmp - min_max[attr][0]) / (min_max[attr][1] - min_max[attr][0])
                    new_data.append(tmp)
                data.append(np.array(new_data))
                if onehot:
                    if row['class'] == 'M':
                        labels.append(np.array([1, 0]))
                    elif row['class'] == 'B':
                        labels.append(np.array([0, 1]))
                    else:
                        raise("unkown class: "+str(row['class']))
                else:
                    labels.append(0 if row['class'] == 'M' else 1)

        data = np.array(data)
        labels = np.array(labels)
        
        self.insert(Resource(data, labels))

class ESR_1_2_Dataset(Dataset):
    
    def __init__(self, esr_source_folder=None, normalized=True, onehot=True):
        super(ESR_1_2_Dataset, self).__init__(normalized=normalized, onehot=onehot, kind='cls')
        data = []
        labels = []

        with open(path.join(esr_source_folder, "data.csv"), "rb") as csvfile:
            file_wrapper = io.TextIOWrapper(csvfile, newline='')
            reader = csv.DictReader(file_wrapper, delimiter=',')
            rows = list(reader)
            min_max = check_min_max(rows)
            for row in rows:
                if str(row['y']) != '1' and  str(row['y']) != '2':
                    continue
                new_data = []
                for attr in reader.fieldnames[1:-1]:
                    tmp = float(row[attr])
                    if normalized:
                        tmp = (tmp - min_max[attr][0]) / (min_max[attr][1] - min_max[attr][0])
                    new_data.append(tmp)
                data.append(np.array(new_data))
                if onehot:
                    labels.append(np.array([1., 0.]) if str(row['y']) != '1' else np.array([0., 1.]))
                else:
                    labels.append(0. if str(row['y']) != '1' else 1.)

        data = np.array(data)
        labels = np.array(labels)
        self.insert(Resource(data, labels))

class HAROnlyTraining_Dataset(Dataset):
    
    def __init__(self, har_source_folder=None, normalized=True, onehot=True):
        super(HAROnlyTraining_Dataset, self).__init__(normalized=normalized, onehot=onehot, kind='cls')
        data = []
        labels = []
        import zipfile
        with zipfile.ZipFile(path.join(har_source_folder, "UCI HAR Dataset.zip"), 'r') as har_zip:
            linesX = []
            linesY = []
            #read
            with har_zip.open("UCI HAR Dataset/train/X_train.txt") as fileX:
                linesX = fileX.readlines()
            with har_zip.open("UCI HAR Dataset/train/y_train.txt") as fileY:
                linesY = fileY.readlines()
            #test
            if len(linesX) != len(linesY):
                raise("{} != {}".format(len(linesX), len(linesY)))
        #from list of string to array
        for line in linesX:
            import re
            lineclean = re.sub(" +", " ", line.decode('ascii').strip())
            values = lineclean.split(" ")
            data.append([float(value) for value in values])
        #append labels
        for line in linesY:
            lineclean = line.decode('ascii').strip()
            if onehot:
                if lineclean == '1':
                    labels.append([1,0,0,0,0,0])
                elif lineclean == '2':
                    labels.append([0,1,0,0,0,0])
                elif lineclean == '3':
                    labels.append([0,0,1,0,0,0])
                elif lineclean == '4':
                    labels.append([0,0,0,1,0,0])
                elif lineclean == '5':
                    labels.append([0,0,0,0,1,0])
                elif lineclean == '6':
                    labels.append([0,0,0,0,0,1])
                else:
                    raise("unkown class: "+str(line))
            else:
                labels.append(float(lineclean)-1.0)
        #as np
        data = np.array(data)
        labels = np.array(labels)
        #norm by numpy
        if normalized:
            data_min = data.min(axis=0)
            data_max = data.max(axis=0)
            data = (data - data_min) / (data_max - data_min)
        #return
        self.insert(Resource(data, labels))
            

class GasSensorArrayDriftDataset(Dataset):

    """Gas Sensor Array Drif dataset."""

    def __init__(self, gass_source_file=None, normalized=True, onehot=True):
        super(GasSensorArrayDriftDataset, self).__init__(
            normalized=normalized, onehot=onehot, kind='cls')

        assert gass_source_file, "You have to specify the GasSensorArrayDrift zip file"

        data = []
        labels = []

        mins = np.zeros(128)
        maxs = np.zeros(128)

        with zipfile.ZipFile(gass_source_file, 'r') as gas_zip:

            for batch_file in gas_zip.namelist():
                if batch_file.find(".dat") != -1:
                    cur_batch = gas_zip.read(batch_file).decode("utf-8")
                    for line in cur_batch.split("\n"):
                        tmp = line.strip().split(" ")
                        if len(tmp) > 1:
                            class_ = int(tmp[0]) - 1
                            cur_data = [float(feature.split(":")[1])
                                        for feature in tmp[1:]]
                            class_ = int(tmp[0]) - 1
                            cur_data = [float(feature.split(":")[1])
                                        for feature in tmp[1:]]
                            ##
                            # Min max
                            for idx, val in enumerate(cur_data):
                                if val < mins[idx]:
                                    mins[idx] = val
                                if val > maxs[idx]:
                                    maxs[idx] = val
                            if len(cur_data) != 128:
                                raise Exception(
                                    "Error on extract gas features")
                            data.append(np.array(cur_data))
                            if onehot:
                                labels.append(np.zeros(6))
                                labels[-1][class_] = 1
                            else:
                                labels.append(class_)

                    if normalized:
                        for record in data:
                            for idx, val in enumerate(record):
                                record[idx] = (val - mins[idx]) / \
                                    (maxs[idx] - mins[idx])

                    self.insert(Resource(np.array(data), np.array(labels)))

                    data = []
                    labels = []

class MNISTDataset(Dataset):

    """MNIST dataset."""

    def __init__(self, mnist_source_folder=None, normalized=True, onehot=True):
        super(MNISTDataset, self).__init__(
            normalized=normalized, onehot=onehot, kind='cls')

        assert mnist_source_folder, "You have to specify in which folder are the mnist files"

        train_images = load_mnist_images(
            path.join(mnist_source_folder, 'train-images-idx3-ubyte.gz'), normalized)
        train_labels = load_mnist_labels(
            path.join(mnist_source_folder, 'train-labels-idx1-ubyte.gz'), onehot)
        test_images = load_mnist_images(
            path.join(mnist_source_folder, 't10k-images-idx3-ubyte.gz'), normalized)
        test_labels = load_mnist_labels(
            path.join(mnist_source_folder, 't10k-labels-idx1-ubyte.gz'), onehot)

        self.insert(Resource(train_images, train_labels))
        self.insert(Resource(test_images, test_labels), 'test')

class NBitParity(Dataset):

    """n-bit parity dataset."""

    def __init__(self, nbit=3, 
                 normalized=True, 
                 onehot=True, 
                 validation_all_values = False, 
                 test_all_values = False,
                 binary = False):
        super(NBitParity, self).__init__(normalized=normalized, onehot=onehot, kind='cls')
        import copy
        def tofuzzyclass(i):
             clazz_min = int(np.min(i))
             clazz_max = int(np.max(i))
             clazz = clazz_max - clazz_min + 1
             size = np.size(i)
             out = np.zeros((size,clazz))
             for k in range(0,size):
                 out[k,int(i[k]) - clazz_min]=1.0
             return out
        pydb_x = []
        for i in range(0,2**nbit):
            pydb_x.append([int(x) for x in list(np.binary_repr(i,width=nbit))])
        db_x = np.array(pydb_x)
        db_y = (np.sum(db_x, axis=1) % 2 != 0).astype(float)

        if binary == False:
             db_y = tofuzzyclass(db_y)
        else:
            to_cols = []
            for x in db_y:
                to_cols.append([x])
            db_y = np.array(to_cols).astype(float)

        self.insert(Resource(db_x,db_y))

        if validation_all_values:
            self.insert(Resource(copy.deepcopy(db_x), copy.deepcopy(db_y)), 'validation')

        if test_all_values:
            self.insert(Resource(copy.deepcopy(db_x), copy.deepcopy(db_y)), 'test')

class FashionMNISTDataset(Dataset):

    """Fashion-MNIST dataset."""

    def __init__(self, fashion_mnist_source_folder=None, normalized=True, onehot=True):
        super(FashionMNISTDataset, self).__init__(
            normalized=normalized, onehot=onehot, kind='cls')

        assert fashion_mnist_source_folder, "You have to specify in which folder are the fashion-mnist files"

        train_images = load_mnist_images(
            path.join(fashion_mnist_source_folder, 'train-images-idx3-ubyte.gz'), normalized)
        train_labels = load_mnist_labels(
            path.join(fashion_mnist_source_folder, 'train-labels-idx1-ubyte.gz'), onehot)
        test_images = load_mnist_images(
            path.join(fashion_mnist_source_folder, 't10k-images-idx3-ubyte.gz'), normalized)
        test_labels = load_mnist_labels(
            path.join(fashion_mnist_source_folder, 't10k-labels-idx1-ubyte.gz'), onehot)

        self.insert(Resource(train_images, train_labels))
        self.insert(Resource(test_images, test_labels), 'test')

def load_mnist_images(file_name, normalized):
    """Load MNIST images."""
    with gzip.open(file_name, 'rb') as mnist_file:
        _, num_images, rows, cols = struct.unpack(
            ">IIII", mnist_file.read(16))
        buf = mnist_file.read(rows * cols * num_images)
        data = np.frombuffer(buf, dtype=np.uint8)
        if normalized:
            data = np.divide(data, 255.)
        data = data.reshape(num_images, rows * cols)
        return data

def load_mnist_labels(file_name, onehot, num_classes=10):
    """Load MNIST image labels."""
    with gzip.open(file_name, 'rb') as mnist_file:
        _, num_labels = struct.unpack(
            ">II", mnist_file.read(8))
        buf = mnist_file.read(num_labels)
        labels = np.frombuffer(buf, dtype=np.uint8)
        if onehot:
            data = np.zeros((num_labels, num_classes))
            data[np.arange(labels.size), labels] = 1
        else:
            data = labels
        return data


READER_LIST = {
    'MNISTDataset': MNISTDataset,
    'FashionMNISTDataset': FashionMNISTDataset,
    'GasSensorArrayDriftDataset': GasSensorArrayDriftDataset,
    'MAGICGammaTelescopeDataset': MAGICGammaTelescopeDataset,
    'QSARDataset': QSARDataset,
    'WDBCDataset': WDBCDataset,
    'ESR_1_2_Dataset' : ESR_1_2_Dataset,
    'HAROnlyTraining_Dataset' : HAROnlyTraining_Dataset,
    'NBitParity': NBitParity
}
