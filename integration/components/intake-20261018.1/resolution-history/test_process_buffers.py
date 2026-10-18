import unittest
from analyze_process_buffers import analyze

P='DWPROCESS v=1 tick=100 device=&A dst=&B src=&C operations=1 flags=0 count=100 hr=0 src_desc_hr=0 src_fvf=274 dst_desc_hr=0 dst_fvf=452 matrix_hr=0 m11=2 m22=3.5 viewport_hr=0 viewport=800x446'
D='DWDRAW v=1 tick=110 device=&A vb=&B kind=5 fvf=452 position=4 indices=90 hr=0 matrix_hr=0 m11=2 m22=3.5 viewport_hr=0 viewport=800x446'

class ProcessBuffers(unittest.TestCase):
    def test_match(self):self.assertEqual(analyze(P+'\n'+D)['near_address_time_matches'],1)
    def test_other_device(self):self.assertEqual(analyze(P+'\n'+D.replace('device=&A','device=&Z'))['near_address_time_matches'],0)
    def test_other_buffer(self):self.assertEqual(analyze(P+'\n'+D.replace('vb=&B','vb=&Z'))['near_address_time_matches'],0)
    def test_stale(self):self.assertEqual(analyze(P+'\n'+D.replace('tick=110','tick=2000'))['near_address_time_matches'],0)
    def test_order(self):self.assertEqual(analyze(D+'\n'+P)['near_address_time_matches'],0)
    def test_failed_process(self):self.assertEqual(analyze(P.replace(' hr=0 ',' hr=-1 ')+'\n'+D)['near_address_time_matches'],0)
    def test_failed_draw(self):self.assertEqual(analyze(P+'\n'+D.replace(' hr=0 ',' hr=-1 '))['near_address_time_matches'],0)
    def test_empty(self):self.assertEqual(analyze('ordinary log')['near_address_time_matches'],0)

if __name__=='__main__':unittest.main()
