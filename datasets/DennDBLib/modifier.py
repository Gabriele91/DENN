"""Here you can find all dataset composers."""
import numpy as np
import random
from .reader import Dataset, Resource
from .analyzer import class_stats

__all__ = ["MODIFIER"]


def add_depth(dataset, depth, target=None, sub_target='attributes'):
    """Add repeated data to insert depth."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"

    if not target:
        for set_ in dataset.data.values():
            for resource in set_:
                resource.repeat(depth, target=sub_target)
    else:
        for resource in getattr(dataset, target):
            resource.repeat(depth, target=sub_target)
    


def simple_shuffle(dataset, target=None, seed=None):
    """Shuffle data."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"

    if not target:
        for set_ in dataset.data.values():
            for resource in set_:
                resource.shuffle(seed)
    else:
        for resource in getattr(dataset, target):
            resource.shuffle(seed)


def extract_to(dataset, from_, to_, num, direct_insert=True):
    """Extract a portion of items to another Resource."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"
    assert from_ == "train" or from_ == "validation" or from_ == "test", "Source set can be 'train', 'validation' or 'test'"
    assert to_ == "train" or to_ == "validation" or to_ == "test", "Target set can be 'train', 'validation' or 'test'"
    
    source = getattr(dataset, from_)
    target = getattr(dataset, to_)
    tmp = Resource(np.array([]), np.array([]))
    
    if isinstance(num, float):
        assert num >= 0 and num <= 1.0, "Not valid extraction percentage [{}]".format(num)
        item_to_extract = int(len(source) * num)
    else:
        item_to_extract = num

    for idx, resource in enumerate(source):
        if item_to_extract < len(resource):
            tmp += resource.extract(range(item_to_extract))
        else:
            tmp += resource.extract(range(len(resource)))
            source.delete(idx)
        item_to_extract -= len(resource)
        if item_to_extract == 0:
            break

    if direct_insert:
        target.insert(tmp)

    return source, target, tmp


def random_copy_to(dataset, from_, to_, num, direct_insert=True):
    """Extract a portion of items to another Resource."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"
    assert from_ == "train" or from_ == "validation" or from_ == "test", "Source set can be 'train', 'validation' or 'test'"
    assert to_ == "train" or to_ == "validation" or to_ == "test", "Target set can be 'train', 'validation' or 'test'"
    
    source = getattr(dataset, from_)
    target = getattr(dataset, to_)
    tmp = Resource(np.array([]), np.array([]))
    #percentage
    if isinstance(num, float):
        assert num >= 0 and num <= 1.0, "Not valid extraction percentage [{}]".format(num)
        item_to_extract = int(len(source) * num)
    else:
        item_to_extract = num
    #resources
    fullsize = 0
    for idx, resource in enumerate(source):
        fullsize += len(resource)
    #factor
    factor = item_to_extract / fullsize
    #get random copy
    for idx, resource in enumerate(source):
        lres = len(resource)
        nsamples = int(lres*factor)
        indexs = random.sample(range(0,lres), nsamples)
        tmp += resource.copy_fields(indexs)

    if direct_insert:
        target.insert(tmp)

    return source, target, tmp


def check_class_ratio(source, target, tolerance):
    """Check class ratio difference."""
    src = np.array(list(source['percentages'].values()))
    trg = np.array(list(target['percentages'].values()))

    return np.sum(np.abs(src - trg)) < tolerance


def check_classes_to_swap(source, target):
    """Check which item classes are better to use on a swap."""
    classes = {}
    for class_, percentage in source['percentages'].items():
        if class_ not in classes:
            classes[class_] = 0.0
        classes[class_] = percentage - target['percentages'][class_]

    diff = sorted([(key, value)
                   for key, value in classes.items()], key=lambda elm: elm[1])

    return int(diff[0][0]), int(diff[-1][0])


def get_first_by_class(source, cls_):
    """Return the index of first element with a certain class."""
    # Don't use enumerate because we have
    # to access the Resource items. So if source
    # is a container it will iterate over resources
    # and not items with enumerate functions, with idenxes
    # instead we have the items.
    for idx in range(len(source)):
        _, labels = source[idx]
        if len(labels) > 1:
            cur_class = np.argmax(labels)
        else:
            cur_class = labels[0]

        if cur_class == cls_:
            return idx


def extract_to_with_class_ratio(dataset, from_, to_, num, class_ratio=None, tolerance=2.0):
    """Extract a portion of items to another Resource with the same class ratio."""
    if not class_ratio:
        class_ratio = class_stats(dataset)
    source, target, extracted = extract_to(
        dataset, from_, to_, num, direct_insert=False)

    extracted_ratio = class_stats(extracted)
    if class_ratio['num_classes'] != extracted_ratio['num_classes']:
        for key in class_ratio['percentages']:
            if key not in extracted_ratio['percentages']:
                extracted_ratio['percentages'][key] = 0.0
                extracted_ratio['counter'][key] = 0.0
        extracted_ratio['num_classes'] = class_ratio['num_classes']

    while not check_class_ratio(class_ratio, extracted_ratio, tolerance):
        # Check class to swap
        remove_cls, add_cls = check_classes_to_swap(
            class_ratio, extracted_ratio)
        # Get element indexes to swap
        idx_to_remove = get_first_by_class(extracted, remove_cls)
        idx_to_add = get_first_by_class(source, add_cls)
        # Swap
        extracted[idx_to_remove], source[idx_to_add] = source[idx_to_add], extracted[idx_to_remove]
        # Update ratio
        extracted_ratio = class_stats(extracted)

    target.insert(extracted)


def merge_resources(dataset, target):
    """Merge all the resource of a specified target."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"
    assert target == "train" or target == "validation" or target == "test", "Target can be 'train', 'validation' or 'test'"

    set_ = getattr(dataset, target)
    tmp = Resource(np.array([]), np.array([]))

    for resource in set_:
        tmp += resource

    # delete all
    for idx in range(len(set_.resources)):
        set_.delete(0)

    set_.insert(tmp)


def split(dataset, target, batch_size=None, n_batch=None):
    """Split a resource in batches."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"
    assert target == "train" or target == "validation" or target == "test", "Target can be 'train', 'validation' or 'test'"
    assert [batch_size is None, n_batch is None].count(
        True) == 1, "You can specify only the size of the batches or the number of the batches"

    set_ = getattr(dataset, target)
    if not batch_size and n_batch:
        batch_size = int(len(set_) / n_batch)
    elif not n_batch and batch_size:
        n_batch = int(len(set_) / batch_size) 
        n_batch += 1 if (len(set_) % batch_size) > 0 else 0

    while any([len(resource) > batch_size for resource in set_]):
        for idx, resource in enumerate(set_):
            if len(resource) > batch_size:
                tmp = resource.split(n_batch)
                set_.delete(idx)
                set_.insert(tmp)


def convert_type(dataset, type_):
    """Convert dataset resource type."""
    assert isinstance(dataset, Dataset), "Not a valid Dataset object"
    assert type_ == 'double' or type_ == 'float', "Output data can be double or float"

    for set_ in dataset.data.values():
        for resource in set_.resources:
            resource.convert_type(type_)


MODIFIER_LIST = {
    'simple_shuffle': simple_shuffle,
    'extract_to': extract_to,
    'random_copy_to': random_copy_to,
    'extract_to_with_class_ratio': extract_to_with_class_ratio,
    'merge_resources': merge_resources,
    'split': split,
    'convert_type': convert_type,
    'add_depth': add_depth
}
