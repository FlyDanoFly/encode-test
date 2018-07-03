def joke():
    return (u'Wenn ist das Nunst\u00fcck git und Slotermeyer? Ja! ... '
            u'Beiherhund das Oder die Flipperwaldt gersput.')


try:
    good_types = (int, long)
except NameError:
    good_types = (int,)


def base36encode(number, alphabet='0123456789abcdefghijklmnopqrstuvwxyz'):
    """
    Converts an integer to a base36 string.
    From wikipedia http://en.wikipedia.org/wiki/Base_36#Python_implementation
    """
    if not isinstance(number, good_types):
        raise TypeError('number must be an integer, not a {} {}'.format(number, type(number)))

    base36 = ''
    sign = ''

    if number < 0:
        sign = '-'
        number = -number

    if 0 <= number < len(alphabet):
        return sign + alphabet[number]

    while number != 0:
        number, i = divmod(number, len(alphabet))
        base36 = alphabet[i] + base36

    return sign + base36
