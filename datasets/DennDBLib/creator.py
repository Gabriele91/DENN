"""DENN dataset creators."""
import binascii
import gzip
import json
import struct
from collections import namedtuple
from os import getcwd, path
import numpy as np
from tqdm import tqdm

from .analyzer import attribute_stats, class_stats
from .modifier import MODIFIER_LIST
from .reader import READER_LIST
from .var import NP2STRUCT

__all__ = [
    "mnist", "fashion_mnist", "qsar",
    "gas_sensor_array_drift",
    "magic_gamma_telescope"
]

Batch = namedtuple('Batch', ['data', 'labels'])


class Dataset(object):

    """DENN dataset object."""

    def __init__(self, file_name):

        self.__file_name = file_name

        with gzip.GzipFile(self.__file_name, mode='rb') as gz_file:
            self.stats = Header(
                "<H5if3I",
                [
                    "version",
                    "n_batch",
                    "n_features",
                    "n_classes",
                    "type",
                    "seed",
                    "train_percentage",
                    "test_offset",
                    "validation_offset",
                    "train_offset"
                ],
                gz_file.read(38)
            )

        # print(self.stats)
        self.type = 'double' if self.stats.type == 2 else 'float'
        self.__dtype = np.float64 if self.stats.type == 2 else np.float32
        self.__elm_size = 8 if self.stats.type == 2 else 4
        self.__size_elm_data = self.stats.n_features * self.__elm_size
        self.__size_elm_label = self.stats.n_classes * self.__elm_size

    def __read_from(self, offset, type_):
        """Read data from offset.

        Args:
            offset: num of bytes to jump
            type: 0 if data, 1 if label
        """
        with gzip.GzipFile(self.__file_name, mode='rb') as gz_file:
            gz_file.seek(offset)
            num_elms = struct.unpack("<I", gz_file.read(4))[0]
            if type_ == 1:
                gz_file.seek(num_elms * self.__size_elm_data, SEEK_CUR)
                data = np.frombuffer(gz_file.read(
                    num_elms * self.__size_elm_label
                ), dtype=self.__dtype)
            else:
                data = np.frombuffer(gz_file.read(
                    num_elms * self.__size_elm_data
                ), dtype=self.__dtype)
            data = data.reshape([
                num_elms,
                self.stats.n_features if type_ == 0 else self.stats.n_classes
            ])
        return data

    @property
    def test_data(self):
        return self.__read_from(self.stats.test_offset, 0)

    @property
    def test_labels(self):
        return self.__read_from(self.stats.test_offset, 1)

    @property
    def validation_data(self):
        return self.__read_from(self.stats.validation_offset, 0)

    @property
    def validation_labels(self):
        return self.__read_from(self.stats.validation_offset, 1)

    @property
    def num_batches(self):
        return self.stats.n_batch

    def batches(self):
        with gzip.GzipFile(self.__file_name, mode='rb') as gz_file:
            gz_file.seek(self.stats.train_offset)

            if self.stats.version == 1:
                for idx in range(self.num_batches):
                    num_batch = struct.unpack("<I", gz_file.read(4))[0]
                    num_elms = struct.unpack("<I", gz_file.read(4))[0]

                    data = np.frombuffer(
                        gz_file.read(num_elms * self.__size_elm_data),
                        dtype=self.__dtype
                    )
                    data = data.reshape([num_elms, self.stats.n_features])

                    labels = np.frombuffer(
                        gz_file.read(num_elms * self.__size_elm_label),
                        dtype=self.__dtype
                    )
                    labels = labels.reshape([num_elms, self.stats.n_classes])

                    yield Batch(data, labels)
            elif self.stats.version == 2:
                for idx in range(self.num_batches):
                    data = [np.frombuffer(
                        gz_file.read(self.__size_elm_data),
                        dtype=self.__dtype
                    )]

                    labels = [np.frombuffer(
                        gz_file.read(self.__size_elm_label),
                        dtype=self.__dtype
                    )]

                    yield Batch(np.array(data), np.array(labels))
            else:
                raise Exception(
                    "Unknown version '{}'".format(self.stats.version))

    def __getitem__(self, index):
        if index > self.num_batches - 1:
            index %= self.num_batches

        with gzip.GzipFile(self.__file_name, mode='rb') as gz_file:
            gz_file.seek(self.stats.train_offset)

            if self.stats.version == 1:
                for idx in range(self.num_batches):
                    num_batch = struct.unpack("<I", gz_file.read(4))[0]
                    num_elms = struct.unpack("<I", gz_file.read(4))[0]

                    # print('Read item ->', num_batch, num_elms)

                    if num_batch == index:
                        break
                    else:
                        gz_file.seek(
                            num_elms * self.__size_elm_data +
                            num_elms * self.__size_elm_label, SEEK_CUR)

                # print('Read item ->', num_elms, self.__size_elm_data)
                data = np.frombuffer(
                    gz_file.read(num_elms * self.__size_elm_data),
                    dtype=self.__dtype
                )
                # print('Read item ->', data.shape)
                data = data.reshape([num_elms, self.stats.n_features])
                # print('Read item ->', data.shape)

                labels = np.frombuffer(
                    gz_file.read(num_elms * self.__size_elm_label),
                    dtype=self.__dtype
                )
                # print('Read item ->', labels.shape)
                labels = labels.reshape([num_elms, self.stats.n_classes])
                # print('Read item ->', labels.shape)

                # print(data[0])
                # print(labels[0])
            elif self.stats.version == 2:
                for idx in range(self.num_batches):
                    if idx == index:
                        break
                    else:
                        gz_file.seek(self.__size_elm_data +
                                     self.__size_elm_label, SEEK_CUR)

                # print('Read item ->', num_elms, self.__size_elm_data)
                data = np.array([np.frombuffer(
                    gz_file.read(self.__size_elm_data),
                    dtype=self.__dtype
                )])
                # print('Read item ->', data.shape)

                labels = np.array([np.frombuffer(
                    gz_file.read(self.__size_elm_label),
                    dtype=self.__dtype
                )])
                # print('Read item ->', labels.shape)

                # print(data[0])
                # print(labels[0])
            else:
                raise Exception(
                    "Unknown version '{}'".format(self.stats.version))

        return Batch(data, labels)


class Header(object):

    """Manage with a simple interface binary headers.

    Example:

        header = Header(
            "<7if",
            [
                # Dataset values
                ("n_batch", 3),
                ("n_features", 4),  # size individual
                ("n_classes", 3),  # size labels
                # Elems
                ('n_train_elms', 120),
                ('n_validation_elms', 15),
                ('n_test_elms', 15),
                # Stats
                ('seed', -1),
                ('train_percentage', 0.8)
            ]
        )

        # get header labels
        print(header.n_classes, header.n_features)
        # set header labels
        print(header.set_label("n_classes", 9))
        # repr of header
        print(header)

        new_header = Header(
            "<7if",
            [
                "n_batch",
                "n_features",
                "n_classes",
                "n_train_elms",
                "n_validation_elms",
                "n_test_elms",
                "seed",
                "train_percentage"
            ],
            header.binary
        )

        print(new_header)
    """

    def __init__(self, fmt, labels, data=None):

        self.__fmt = fmt
        self.__data = data
        self.__p_data = labels
        self.__out_size = 42

        if not data:
            self.__p_data = labels
            self.__data = struct.pack(
                self.__fmt,
                *[value for label, value in self.__p_data]
            )
        else:
            self.__data = data
            for idx, value in enumerate(struct.unpack(self.__fmt, self.__data)):
                self.__p_data[idx] = (labels[idx], value)

    def __len__(self):
        return struct.calcsize(self.__fmt)

    @property
    def binary(self):
        """Get binary of python data."""
        return self.__data

    def __getattr__(self, name):
        for label, value in self.__p_data:
            if label == name:
                return value
        raise AttributeError("'{}' is not a label of this header!".format(
            name
        ))

    def set_label(self, name, new_value):
        """Change an header label value."""
        for idx, _ in enumerate(self.__p_data):
            if self.__p_data[idx][0] == name:
                self.__p_data[idx] = (name, new_value)

                self.__data = struct.pack(
                    self.__fmt,
                    *[value for label, value in self.__p_data]
                )

                return self.__p_data[idx]

        raise Exception("'{}' is not a label of this header!".format(
            name
        ))

    def __repr__(self):
        byte_per_line = 8

        string = "+----------- HEADER ".ljust(self.__out_size, "-") + '+\n'
        format_ = "| format string: '{}'".format(self.__fmt)
        string += format_.ljust(self.__out_size, " ") + '|\n'
        string += "+".ljust(self.__out_size, "-") + '+\n'

        for label, value in self.__p_data:
            cur_data = "| {}: {}".format(label, value)
            string += cur_data.ljust(self.__out_size, " ") + '|\n'

        string += "+".ljust(self.__out_size, "-") + '+\n'

        data = binascii.b2a_hex(self.__data)
        counter = 0
        cur_data = ''

        for idx in range(0, len(data), 4):
            if counter == 0:
                cur_data += "| "
            elif counter == byte_per_line:
                counter = 0
                string += cur_data.ljust(self.__out_size, " ") + '|\n'
                cur_data = "| "

            tmp = [data[idx + cur_i] for cur_i in range(4)]
            if all([isinstance(elm, int) for elm in tmp]):
                cur_data += "{} ".format("".join([chr(elm) for elm in tmp]))
            else:
                cur_data += "{} ".format("".join(tmp))
            counter += 2

        string += "+".ljust(self.__out_size, "-") + '+\n'

        return string


class Action(object):

    """An action that have to be applied to the dataset."""

    def __init__(self, type_, name, args, kwargs):
        self.__type = type_
        self.__args = args
        self.__kwargs = kwargs
        self.__name = name
        if type_ == 'modifier':
            self.__func = MODIFIER_LIST[name]
        else:
            raise Exception("Unkown action type '{}'".format(type_))

    def __call__(self, *args, **kwargs):
        tmp_args = args + self.__args
        tmp_kwargs = dict((key, value) for key, value in self.__kwargs.items())
        tmp_kwargs.update(kwargs)
        return self.__func(*tmp_args, **tmp_kwargs)


class Generator(object):

    """DENN dataset generator."""

    def __init__(self, dataset, dataset_params, actions, out_type='double', depth=1, version=1):
        assert out_type == 'double' or out_type == 'float', "Output data can be double or float"

        self.dataset = READER_LIST[dataset](**dataset_params)
        self.actions = []
        self.header = None
        self.out_type = out_type
        self.depth = depth
        self.version = version
        self.register_action(actions)

    def register_action(self, actions):
        """Register an action."""
        if isinstance(actions, list):
            for action in actions:
                self.actions.append(Action(*action))
        else:
            self.actions.append(Action(*actions))

    def execute_actions(self):
        """Execute all action in list."""
        for action in self.actions:
            action(self.dataset)

    def gen_header(self):
        """Generate DENN dataset header."""
        n_features = len(
            self.dataset[0][0][0]) if self.depth > 1 else len(self.dataset[0][0])
        n_classes = len(self.dataset[0][1])
        self.header = Header(
            "<H5if3I",
            [
                ("version", self.version),
                # Dataset values
                ("n_batch", len(self.dataset.train.resources)),
                ("n_features", n_features),  # size attributes
                ("n_classes", n_classes),  # size labels
                ##
                # Type of values:
                #   1 -> float
                #   2 -> double
                ('type', 2 if self.out_type == "double" else 1),
                # Stats
                ('seed', -1),
                ('train_percentage', -1),
                # Offset
                ('test_offset', 0),
                ('validation_offset', 0),
                ('train_offset', 0)
            ]
        )
        return self

    def get_stats(self):
        """Generate dataset statistics."""
        return {
            'class_stats': class_stats(self.dataset),
            'attribute_stats': attribute_stats(self.dataset)
        }

    def save_stats(self, out_filename, dest_folder):
        """Stave statistics of the dataset."""
        with open(path.join(dest_folder, out_filename + ".stat.json"), "w") as out_stats:
            json.dump(self.get_stats(), out_stats, indent=2)

    def save(self, out_filename, dest_folder):
        """Store the dataset."""
        self.gen_header()

        with gzip.GzipFile(path.join(dest_folder, out_filename + ".gz"), mode='wb') as gz_file:

            len_test_attributes = len(
                self.dataset.test.resources[0].attributes.flat)
            len_test_outputs = len(
                self.dataset.test.resources[0].outputs.flat)
            len_validation_attributes = len(
                self.dataset.validation.resources[0].attributes.flat)
            len_validation_outputs = len(
                self.dataset.validation.resources[0].outputs.flat)

            print("+++ Calculate test size")
            test_type = self.dataset.test[0][0].dtype if len(
                self.dataset.test) > 0 else np.dtype('f')
            test_size = struct.calcsize("{}{}".format(
                len_test_attributes,
                NP2STRUCT[test_type])
            ) + struct.calcsize("{}{}".format(
                len_test_outputs,
                NP2STRUCT[test_type])
            )

            print("+++ Calculate validation size")
            validation_type = self.dataset.validation[0][0].dtype if len(
                self.dataset.validation) > 0 else np.dtype('f')
            validation_size = struct.calcsize("{}{}".format(
                len_validation_attributes,
                NP2STRUCT[validation_type])
            ) + struct.calcsize("{}{}".format(
                len_validation_outputs,
                NP2STRUCT[validation_type])
            )
            # size of headers
            test_header_size = 4 + (4 if self.depth > 1 else 0)
            validation_header_size = 4 + (4 if self.depth > 1 else 0)
            #train_header_size = 4 + 4 + (4 if self.depth  > 1 else 0)

            print('+++')
            print('+++ Samples train', len(self.dataset.train))
            print('+++ Samples validation', len(self.dataset.validation))
            print('+++ Samples test', len(self.dataset.test))
            print('+++')
            print('+++ Size header', len(self.header))
            print('+++ Size validation', validation_size)
            print('+++ Size test', test_size)

            print("+++ Update offsets in header")
            self.header.set_label("test_offset",
                                  len(self.header))  # size header
            # size header + test
            self.header.set_label("validation_offset",
                                  len(self.header) + test_size + test_header_size)
            # size header + test + validation
            self.header.set_label("train_offset",
                                  len(self.header) + test_header_size + test_size + validation_header_size + validation_size)
            # print(header)

            print("+++ Write header")
            gz_file.write(self.header.binary)
            # print(gz_file.tell())

            ##
            # Block ver. 1
            # +----------------------------+
            # | batch index (unsigned long)|
            # | (only if is train set)     |
            # +----------------------------+
            # | num. elems (unsigned long) |
            # +----------------------------+
            # | data                       |
            # +----------------------------+
            # | labels                     |
            # +----------------------------+

            ##
            # Block ver. 2
            # +----------------------------+
            # | num. elems (unsigned long) |
            # +----------------------------+
            # | data                       |
            # +----------------------------+
            # | labels                     |
            # +----------------------------+

            ##
            # Block ver. 3
            # +----------------------------+
            # | batch index (unsigned long)|
            # | (only if is train set)     |
            # +----------------------------+
            # | num. elems (unsigned long) |
            # +----------------------------+
            # | depth (unsigned long)      |
            # +----------------------------+
            # | data * depth               |
            # +----------------------------+
            # | labels                     |
            # +----------------------------+

            for name, set_ in reversed(self.dataset.data.items()):
                print("+++ Writing {} set".format(name))
                for idx, resource in enumerate(tqdm(set_.resources)):
                    # batch index
                    if name == "train" and self.version != 2:
                        gz_file.write(struct.pack("<I", idx))
                    # depth
                    if self.version == 3:
                        # num. elems
                        gz_file.write(struct.pack("<I", len(resource.outputs)))
                        gz_file.write(struct.pack("<I", self.depth))
                        # data * depth
                        gz_file.write(resource.to_bin())
                    else:
                        # num. elems
                        gz_file.write(struct.pack("<I", len(resource)))
                        # data or labels, depends on current set_
                        gz_file.write(resource.to_bin())

        print("+! Dataset {}.gz completed!".format(out_filename))


def mnist(out_filename, source_folder, dest_folder=getcwd(), depth=1,
          one_hot=True, normalized=True, out_type="float", balanced_classes=False,
          n_batch=None, batch_size=None, validation_size=2000, validation_as_copy=False, save_stats=False):
    """Create a mnist dataset for DENN."""

    dataset_params = {
        'mnist_source_folder': source_folder,
        'normalized': normalized,
        'onehot': one_hot
    }

    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'validation', validation_size), {}),
        ('modifier', 'split', ('train',), { 'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'extract_to_with_class_ratio', args, kwargs)

    if validation_as_copy:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'random_copy_to', args, kwargs)

    if depth > 1:
        actions.append(('modifier', 'add_depth', (depth,), {}))

    version = 3 if depth > 1 else 1

    generator = Generator('MNISTDataset', dataset_params, actions,
                          version=version, out_type=out_type, depth=depth)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

    # print(generator.header)

    # print(len(generator.dataset.train))
    # print(len(generator.dataset.validation))
    # print(len(generator.dataset.test))

    # for resource in generator.dataset.train.resources:
    #     print(len(resource))

def nbit_parity(out_filename, nbit=None, dest_folder=getcwd(), depth=1,
          one_hot=True, normalized=True, out_type="float",
          balanced_classes=False, n_batch=None, batch_size=None, 
          validation_all_values = False, 
          validation_as_copy = False,
          test_all_values = False, 
          validation_size=0.1, test_size = 0.1, 
          save_stats=False
          ):
    """Create a mnist dataset for DENN."""

    dataset_params = {
        'nbit': nbit or 8,
        'normalized': normalized,
        'onehot': one_hot,
        'validation_all_values' : validation_all_values,
        'test_all_values' : test_all_values,
    }
    #first part
    actions = [
        ('modifier', 'merge_resources', ('train',), {}),
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
    ]
    #copy all db
    if not test_all_values:
        actions.append(('modifier', 'extract_to', ('train', 'test', test_size), {}))
    if not validation_all_values:
        val_action = "extract_to" if not validation_as_copy else 'random_copy_to'
        actions.append(('modifier', val_action, ('train', 'validation', validation_size), {}))
    #end part
    actions = actions + [
        ('modifier', 'split', ('train',), {
         'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'extract_to_with_class_ratio', args, kwargs)

    if depth > 1:
        actions.append(('modifier', 'add_depth', (depth,), {}))

    version = 3 if depth > 1 else 1

    generator = Generator('NBitParity', dataset_params, actions, version=version, out_type=out_type, depth=depth)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def fashion_mnist(out_filename, source_folder, dest_folder=getcwd(),
                  one_hot=True, normalized=True, out_type="float", balanced_classes=False,
                  n_batch=None, batch_size=None, validation_size=2000, validation_as_copy=False, save_stats=False):
    """Create a fashion mnist dataset for DENN."""

    dataset_params = {
        'fashion_mnist_source_folder': source_folder,
        'normalized': normalized,
        'onehot': one_hot
    }

    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'validation', validation_size), {}),
        ('modifier', 'split', ('train',), {
         'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'extract_to_with_class_ratio', args, kwargs)

    if validation_as_copy:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'random_copy_to', args, kwargs)

    generator = Generator('FashionMNISTDataset', dataset_params,
                          actions, out_type=out_type)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def gas_sensor_array_drift(out_filename, source_file=None, dest_folder=getcwd(),
                           one_hot=True, normalized=True, out_type="float", balanced_classes=False,
                           n_batch=None, batch_size=None, validation_size=400, test_size=1800, save_stats=False):
    """Create a gas sensor array drift dataset for DENN."""

    dataset_params = {
        'gass_source_file': source_file or path.join("..", "source_data", "GasSensorArrayDrift.zip"),
        'normalized': normalized,
        'onehot': one_hot
    }

    actions = [
        ('modifier', 'merge_resources', ('train',), {}),
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'validation', validation_size), {}),
        ('modifier', 'extract_to', ('train', 'test', test_size), {}),
        ('modifier', 'split', ('train',), { 'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'extract_to_with_class_ratio', args, kwargs)

    generator = Generator('GasSensorArrayDriftDataset', dataset_params,
                          actions, out_type=out_type)

    # print(len(generator.dataset))
    # print(len(generator.dataset.train.resources[0]))
    # print(len(generator.dataset.train.resources[1]))
    # print(len(generator.dataset.train.resources[9]))

    generator.execute_actions()

    # print(len(generator.dataset.train.resources[0]))
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def magic_gamma_telescope(out_filename, source_folder=None, dest_folder=getcwd(),
                          one_hot=True, normalized=True, out_type="float", balanced_classes=False,
                          n_batch=None, batch_size=None, validation_size=570, test_size=3040, save_stats=False):
    """Create a gas sensor array drift dataset for DENN."""

    dataset_params = {
        'magic_source_folder': source_folder or path.join("..", "source_data", "magic"),
        'normalized': normalized,
        'onehot': one_hot
    }

    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'validation', validation_size), {}),
        ('modifier', 'extract_to', ('train', 'test', test_size), {}),
        ('modifier', 'split', ('train',), {
         'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'extract_to_with_class_ratio', args, kwargs)

    generator = Generator('MAGICGammaTelescopeDataset', dataset_params,
                          actions, out_type=out_type)

    # print(len(generator.dataset))
    # print(len(generator.dataset.train.resources[0]))
    # print(len(generator.dataset.train.resources[1]))
    # print(len(generator.dataset.train.resources[9]))

    generator.execute_actions()

    # print(len(generator.dataset.train.resources[0]))
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def qsar(out_filename, source_folder=None, dest_folder=getcwd(),
         one_hot=True, normalized=True, out_type="float", balanced_classes=False,
         n_batch=None, batch_size=None, validation_size=30, test_size=160, save_stats=False):
    """Create a gas sensor array drift dataset for DENN."""

    dataset_params = {
        'qsar_source_folder': source_folder or path.join("..", "source_data", "qsar"),
        'normalized': normalized,
        'onehot': one_hot
    }

    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'validation', validation_size), {}),
        ('modifier', 'extract_to', ('train', 'test', test_size), {}),
        ('modifier', 'split', ('train',), {
         'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (
                    type_, 'extract_to_with_class_ratio', args, kwargs)

    generator = Generator('QSARDataset', dataset_params,
                          actions, out_type=out_type)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def wdbc(out_filename, source_folder, dest_folder=getcwd(),
         one_hot=True, normalized=True, out_type="float", balanced_classes=False,
         n_batch=None, batch_size=None, validation_size=30, validation_as_copy=False, test_size=160, save_stats=False):
    """Create a wdbc dataset for DENN."""

    dataset_params = {
        'wdbc_source_folder': source_folder,
        'normalized': normalized,
        'onehot': one_hot
    }
    val_action = "extract_to" if not validation_as_copy else 'random_copy_to'
    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'test', test_size), {}),
        ('modifier', val_action, ('train', 'validation', validation_size), {}),
        ('modifier', 'split', ('train',), { 'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (type_, 'extract_to_with_class_ratio', args, kwargs)

    generator = Generator('WDBCDataset', dataset_params,
                          actions, out_type=out_type)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def esr_1_2(out_filename, source_folder, dest_folder=getcwd(),
         one_hot=True, normalized=True, out_type="float", balanced_classes=False,
         n_batch=None, batch_size=None, validation_size=30, validation_as_copy=False, test_size=160, save_stats=False):
    """Create a esr dataset for DENN."""

    dataset_params = {
        'esr_source_folder': source_folder,
        'normalized': normalized,
        'onehot': one_hot
    }
    val_action = "extract_to" if not validation_as_copy else 'random_copy_to'
    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'test', test_size), {}),
        ('modifier',  val_action, ('train', 'validation', validation_size), {}),
        ('modifier', 'split', ('train',), {
         'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (type_, 'extract_to_with_class_ratio', args, kwargs)


    generator = Generator('ESR_1_2_Dataset', dataset_params, actions, out_type=out_type)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator

def har_only_training(out_filename, source_folder, dest_folder=getcwd(),
         one_hot=True, normalized=True, out_type="float", balanced_classes=False,
         n_batch=None, batch_size=None, validation_size=30, validation_as_copy=False, test_size=160, save_stats=False):
    """Create a har dataset for DENN."""

    dataset_params = {
        'har_source_folder': source_folder,
        'normalized': normalized,
        'onehot': one_hot
    }
    val_action = "extract_to" if not validation_as_copy else 'random_copy_to'
    actions = [
        ('modifier', 'simple_shuffle', (), {'target': "train"}),
        ('modifier', 'extract_to', ('train', 'test', test_size), {}),
        ('modifier',  val_action, ('train', 'validation', validation_size), {}),
        ('modifier', 'split', ('train',), {
         'batch_size': batch_size, 'n_batch': n_batch}),
        ('modifier', 'convert_type', (out_type,), {})
    ]

    if balanced_classes:
        for idx, (type_, action, args, kwargs) in enumerate(actions):
            if action == "extract_to":
                actions[idx] = (type_, 'extract_to_with_class_ratio', args, kwargs)

    generator = Generator('HAROnlyTraining_Dataset', dataset_params, actions, out_type=out_type)
    generator.execute_actions()
    generator.save(out_filename, dest_folder)
    if save_stats:
        generator.save_stats(out_filename, dest_folder)

    return generator