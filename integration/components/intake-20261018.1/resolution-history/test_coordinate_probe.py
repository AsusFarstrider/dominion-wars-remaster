import unittest
from analyze_coordinate_probe import analyze
STATUS='DWAB_STATUS v=1 id=0 count=100 set_a=0 process_a=0 read_a=0 set_b=0 process_b=0 read_b=0 restored=1'
SUMMARY='DWAB_SUMMARY v=1 id=0 finite=100 count=100 max_dx=25 max_dy=0 max_dz=0 max_drhw=0'
VERTEX='DWAB_VERTEX v=1 id=0 index=0 ax=500 ay=200 bx=475 by=200'
class Coordinates(unittest.TestCase):
    def test_success(self):
        p=analyze('\n'.join((STATUS,SUMMARY,VERTEX)))['probes'][0]
        self.assertTrue(p['measurement_valid']);self.assertTrue(p['horizontal_output_changed'])
        self.assertEqual(p['sample_max_error_expected_075_about_x400'],0)
    def test_read_failure(self):self.assertFalse(analyze((STATUS+'\n'+SUMMARY).replace('read_b=0','read_b=-1'))['probes'][0]['measurement_valid'])
    def test_restoration_failure(self):self.assertFalse(analyze(STATUS+'\n'+SUMMARY+'\nDWAB_FATAL v=1 projection_restore_failed')['probes'][0]['measurement_valid'])
    def test_nonfinite(self):self.assertFalse(analyze(STATUS+'\n'+SUMMARY.replace('max_dx=25','max_dx=nan'))['probes'][0]['measurement_valid'])
    def test_creation_only(self):self.assertFalse(analyze('DWAB_STATUS v=1 id=0 create_hr=-1')['probes'][0]['measurement_valid'])
    def test_no_records(self):self.assertEqual(analyze('normal log')['probes'],{})
if __name__=='__main__':unittest.main()
