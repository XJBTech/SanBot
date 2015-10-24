from unittest import TestCase
from util.boyer_moore_search import z_array
from util.boyer_moore_search import boyer_moore, BoyerMoore

__author__ = 'Yifei'


class TestBoyerMooreSearch(TestCase):
    def test_z_array(self):
        # Test Case 0
        s = 'aabcaabxaaaz'
        n = len(s)
        z = z_array(s)
        self.assertListEqual([n, 1, 0, 0, 3, 1, 0, 0, 2, 2, 1, 0], z, "Test Case 1: " + s)

        s = 'aaaaaa'
        n = len(s)
        z = z_array(s)
        self.assertListEqual([n, 5, 4, 3, 2, 1], z, "Test Case 2: " + s)

        s = 'aabaacd'
        n = len(s)
        z = z_array(s)
        self.assertListEqual([n, 1, 0, 2, 1, 0, 0], z, "Test Case 3: " + s)

        s = 'abababab'
        n = len(s)
        z = z_array(s)
        self.assertListEqual([n, 0, 6, 0, 4, 0, 2, 0], z, "Test Case 4: " + s)

    def test_boyer_moore(self):
        t = [1, 2, 3, 1, 2, 3, 1, 2, 3]
        p = [1]
        p_bm = BoyerMoore(p)
        occurrences = boyer_moore(p, p_bm, t)
        self.assertListEqual([0, 3, 6], occurrences, "Test Case 1: " + str(p) + " in " + str(t))

        t = [1, 2, 3, 1, 2, 3, 1, 2, 3]
        p = [1, 2]
        p_bm = BoyerMoore(p)
        occurrences = boyer_moore(p, p_bm, t)
        self.assertListEqual([0, 3, 6], occurrences, "Test Case 2: " + str(p) + " in " + str(t))

        t = [1, 2, 3, 1, 2, 3, 1, 2, 3]
        p = [1, 2, 3]
        p_bm = BoyerMoore(p)
        occurrences = boyer_moore(p, p_bm, t)
        self.assertListEqual([0, 3, 6], occurrences, "Test Case 3: " + str(p) + " in " + str(t))

        t = [1, 2, 3, 4, 5]
        p = [6, 6, 6]
        p_bm = BoyerMoore(p)
        occurrences = boyer_moore(p, p_bm, t)
        self.assertListEqual([], occurrences, "test case 4: " + str(p) + " in " + str(t))

        t = [1, 2, 3, 4, 5]
        p = [6]
        p_bm = BoyerMoore(p)
        occurrences = boyer_moore(p, p_bm, t)
        self.assertListEqual([], occurrences, "test case 5: " + str(p) + " in " + str(t))
