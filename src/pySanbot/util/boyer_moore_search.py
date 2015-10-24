__author__ = 'Yifei'


def z_array(s):
    """
    Z-algorithm used in BM-Search
    :param s: the string from which to extract
    :return: a list of the length of prefix-substring
    """
    assert len(s) > 1

    n = len(s)
    z = [0] * n
    z[0] = n
    l, r = 0, 0
    for i in range(1, n):
        if i > r:
            # i > r, i is on the right side of r
            # reset position of l and r because s[l..r] at least starts at s[i]
            l, r = i, i
            while r < n and s[r - l] == s[r]:
                # compare S[0...] until r hits the end
                r += 1
            # restore r back to the location (we're off by one)
            r -= 1
            # the maximum prefix-substring will have the length of r - l +1
            #  because r stops at first non-matching letter after l
            z[i] = r - l + 1
        else:
            # i <= R, i not on the right side
            k = i - l
            if z[k] < r - i + 1:
                # we have at most this many matches
                z[i] = z[k]
            else:
                l = i
                # we could (possibly) still match some after s[i]
                # therefore, we continue with s[r], which corresponds to s[r-l],
                # the first character matched in s
                while r < n and s[r - l] == s[r]:
                    r += 1
                # restore r back to the location (same, we're off by one)
                r -= 1
                z[i] = r - l + 1
    return z


def n_array(s):
    """
    Generate the N array from Z array, Theorem 0.2.2
    Nj(P) is the length of longest suffix of substring P[1..j], also a suffix of P
    Zj(P) is the length of longest prefix of substring P[1..j]
    Nj(P) = Z_(n-j+1) (reverse(P))
    :param s: the string
    :return: the reverse of Z( the reverse of s )
    """
    return z_array(s[::-1])[::-1]


def big_l_prime_array(p, n):
    """
    Generate L' array using pattern and N array, Theorem 0.2.2
    L'[i] = largest index j less than n such that N[j] = |P[i:]|
    :param p: the pattern
    :param n: the N array
    :return: the L' array
    """
    lp = [0] * len(p)
    for j in range(len(p)):
        i = len(p) - n[j]
        if i < len(p):
            lp[i] = j + 1
    return lp


def big_l_array(p, lp):
    """
    Generate L array using pattern and L' array, Theorem 0.2.2, see proof
    :param p: the pattern
    :param lp: the L' array
    :return: the L array
    """
    l = [0] * len(p)
    l[1] = lp[1]
    for i in range(2, len(p)):
        l[i] = max(l[i - 1], lp[i])
    return l


def small_l_prime_array(n):
    """
    Generate l' array using N array
    l'[i] is the length of largest suffix of P[i..n] that is also a prefix of P || 0
    :param n: the N array
    :return: l' array
    """
    small_lp = [0] * len(n)
    for i in range(len(n)):
        if n[i] == i + 1:  # the longest suffix matching prefix
            small_lp[len(n) - i - 1] = i + 1
    for i in range(len(n) - 2, -1, -1):
        if small_lp[i] == 0: # copy the right side if no such suffix-prefix matching
            small_lp[i] = small_lp[i + 1]
    return small_lp


def good_suffix_table(p):
    """
    Return the tables needed to apply good suffix rule
    :param p: the pattern
    :return: good suffix table
    """
    n = n_array(p)
    lp = big_l_prime_array(p, n)
    return lp, big_l_array(p, lp), small_l_prime_array(n)


def good_suffix_mismatch(i, big_l_prime, small_l_prime):
    """
    Given a mismatch at offset i and given L/L' and l'
    :param i:  the position of mismatch
    :param big_l_prime: L'
    :param small_l_prime: l'
    :return: the amount of shift
    """
    length = len(big_l_prime)
    assert i < length
    if i == length - 1:
        # no place to shift any more
        return 0
    i += 1  # i points to leftmost matching position of P
    # L'[i] > 0, there are something matching the suffix of this P
    if big_l_prime[i] > 0:
        # move to the place by aligning the substring (matched with suffix) in P
        #  with i
        return length - big_l_prime[i]
    # L'[i] = 0
    # do not have a suffix-matched substring, aligning the suffix-prefix-match
    #  if exists otherwise, we will move all the way left
    return length - small_l_prime[i]


def good_suffix_match(small_l_prime):
    """
    Given a full match of P to T, return the amount to shift as determined by
    good suffix rule
    :param small_l_prime: l' array
    :return: the amount of shift when fully matched
    """
    return len(small_l_prime) - small_l_prime[1]


def dense_bad_char_table(p, alphabet):
    table = []
    nxt = [0] * len(alphabet)
    for i in range(0, len(p)):
        c = p[i]
        assert c in alphabet
        table.append(nxt[:])
        nxt[c] = i + 1
    return table


class BoyerMoore(object):

    def __init__(self, p):
        if len(p) == 1:
            return
        self.p = p
        self.alphabet = []
        for i in range(256):
            self.alphabet.append(i)

        # make bad character
        self.bad_char = dense_bad_char_table(p, self.alphabet)

        _, self.big_l, self.small_l_prime = good_suffix_table(p)

    def bad_character_rule(self, i, c):
        return i - (self.bad_char[i][c] - 1)

    def good_suffix_rule(self, i):
        length = len(self.big_l)
        if i == length - 1:
            return 0
        i += 1  # i points to leftmost matching position
        if self.big_l[i] > 0:
            return length - self.big_l[i]
        return length - self.small_l_prime[i]

    def match_skip(self):
        return len(self.small_l_prime) - self.small_l_prime[1]


def boyer_moore(p, p_bm, t):
    i = 0
    occurrences = []
    if len(p) == 1:
        for i in range(len(t)):
            if t[i] == p[0]:
                occurrences.append(i)
        return occurrences
    while i < len(t) - len(p) + 1:
        # default shift
        shift = 1
        mismatched = False
        for j in range(len(p) - 1, -1, -1):
            if not p[j] == t[i + j]:
                skip_bc = p_bm.bad_character_rule(j, t[i+j])
                skip_gs = p_bm.good_suffix_rule(j)
                shift = max(shift, skip_bc, skip_gs)
                mismatched = True
                break
        if not mismatched:
            occurrences.append(i)
            skip_gs = p_bm.match_skip()
            shift = max(shift, skip_gs)
        # update position by amount of shift
        i += shift
    return occurrences

