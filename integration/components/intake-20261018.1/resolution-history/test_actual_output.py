import unittest
from analyze_actual_output import analyze
from test_coordinate_probe import STATUS,SUMMARY
PREFIX=STATUS+'\n'+SUMMARY+'\nDWACTUAL_STATUS v=1 id=0 read_hr=0 count=100 vb=&1\n'
TAIL='DWACTUAL_SUMMARY v=1 id=0 finite=100 count=100 max_dx_a=10 max_dy_a=0 max_dx_b=0 max_dy_b=0'
class Actual(unittest.TestCase):
    def result(self,t):return analyze(t)['actual'][0]
    def test_b(self):self.assertEqual(self.result(PREFIX+TAIL)['xy_match'],'B_1.5')
    def test_a(self):self.assertEqual(self.result(PREFIX+TAIL.replace('max_dx_a=10','max_dx_a=0').replace('max_dx_b=0','max_dx_b=10'))['xy_match'],'A_2.0')
    def test_neither(self):self.assertEqual(self.result(PREFIX+TAIL.replace('max_dx_b=0','max_dx_b=1'))['xy_match'],'neither')
    def test_failed_read(self):self.assertFalse(self.result((PREFIX+TAIL).replace('read_hr=0','read_hr=-1'))['measurement_valid'])
    def test_bad_count(self):self.assertFalse(self.result(PREFIX+TAIL.replace('finite=100','finite=99'))['measurement_valid'])
    def test_no_data(self):self.assertEqual(analyze('normal log')['actual'],{})
if __name__=='__main__':unittest.main()
