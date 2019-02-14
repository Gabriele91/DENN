"""Data analyzers."""

import numpy as np

from .reader import Dataset, Resource


__all__ = ["class_stats", "attribute_stats"]


def class_stats(data):
    """Get number of items per class and percentages."""
    assert isinstance(data, Dataset) or isinstance(
        data, Resource), "Not a valid object"

    class_counter = {}
    class_percentages = {}

    for _, labels in data:
        if len(labels) > 1:
            class_ = str(np.argmax(labels))
        else:
            class_ = str(labels)

        if class_ not in class_counter:
            class_counter[class_] = 0
        class_counter[class_] += 1

    for class_, count in class_counter.items():
        class_percentages[class_] = (float(count) / float(len(data))) * 100.

    return {
        'num_classes': len(class_counter),
        'counter': class_counter,
        'percentages': class_percentages
    }


def attribute_stats(data):
    """Get item attributes stats."""
    assert isinstance(data, Dataset) or isinstance(
        data, Resource), "Not a valid object"

    ii64 = np.iinfo(np.int64)
    res = {
        'max': ii64.min,
        'min': ii64.max,
        'variance': None,
        'mean': None,
        'coefficient_of_variation': None
    }

    values_sum = 0
    for attributes, _ in data:
        values_sum += np.sum(attributes)
        min_ = np.amin(attributes)
        max_ = np.amax(attributes)
        if max_ > res['max']:
            res['max'] = max_
        if min_ < res['min']:
            res['min'] = min_

    mean = res['mean'] = values_sum / float(len(data))

    variance_tmp_sum = 0.0
    for attributes, _ in data:
        tmp = np.subtract(attributes, mean)
        tmp = np.square(tmp)
        variance_tmp_sum += np.sum(tmp)

    res['variance'] = variance_tmp_sum / float(len(data))
    res['coefficient_of_variation'] = res['variance'] / res['mean']

    # Convert to python float for json serialization
    for key, item in res.items():
        res[key] = float(item)

    return res
