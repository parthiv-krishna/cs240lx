def to_int(a):
    """Converts a byte array to an integer.

    Args:
        a (bytes)

    Returns:
        int: a
    """
    return int.from_bytes(a, byteorder="big")

def to_bytes(i, length):
    """Converts an integer to a byte array.

    Args:
        i (int)
        length (int)

    Returns:
        bytes: i
    """
    return i.to_bytes(length, byteorder="big", signed=False)

def to_hex(a):
    """Converts a byte array to a hex string.

    Args:
        a (bytes)

    Returns:
        str: a
    """
    return format(to_int(a), 'x')

def AND(a, b):
    """Performs bitwise and on two given byte arrays.

    Args:
        a (bytes) 
        b (bytes)

    Returns:
        bytes: a & b
    """
    result = to_int(a) & to_int(b)
    return to_bytes(result, max(len(a), len(b)))

def OR(a, b):
    """Performs bitwise or on two given byte arrays.

    Args:
        a (bytes) 
        b (bytes)

    Returns:
        bytes: a | b
    """
    result = to_int(a) | to_int(b)
    return to_bytes(result, max(len(a), len(b)))

def NOT(a):
    """Performs bitwise not on a given byte array.

    Args:
        a (bytes)

    Returns:
        bytes: ~a
    """
    ones = to_int(bytes([0xFF for i in range(len(a))]))
    result = to_int(a) ^ ones # a ^ 1 = ~a
    return to_bytes(result, len(a))

def FFS(a):
    """Finds index of least significant set bit.

    Args:
        a (bytes): _description_

    Returns:
        int: the index (0 for LSB, 1 for 2nd LSB, etc.) or -1 if no bits are set
    """
    max_idx = len(a) * 8
    a = to_int(a) # use int value for and
    for i in range(max_idx):
        if (a & (1 << i)) != 0:
            return i
    return -1